from flask import Flask, render_template, request, jsonify, send_file
import os
from werkzeug.utils import secure_filename
from pathlib import Path
import tempfile
import subprocess
import logging
import sys
import shutil
import re

# Configure logging
logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'uploads')
app.config['MAX_CONTENT_LENGTH'] = 16 * 1024 * 1024  # 16MB max file size
app.config['ALLOWED_EXTENSIONS'] = {'cpp', 'h', 'hpp', 'cc', 'cxx'}

# Ensure upload directory exists
os.makedirs(app.config['UPLOAD_FOLDER'], exist_ok=True)
logger.info(f"Upload directory: {app.config['UPLOAD_FOLDER']}")

def extract_class_info(file_content):
    """Extract class information from C++ code."""
    classes = []
    current_class = None
    relationships = []
    
    # Split the file into lines
    lines = file_content.split('\n')
    
    for line in lines:
        # Clean up the line
        line = line.strip()
        
        # Skip empty lines and preprocessor directives
        if not line or line.startswith('#'):
            continue
            
        # Look for class declarations
        class_match = re.match(r'class\s+(\w+)(?:\s*:\s*(.+))?', line)
        if class_match:
            class_name = class_match.group(1)
            inheritance = class_match.group(2)
            
            current_class = {
                'name': class_name,
                'methods': [],
                'attributes': [],
                'inheritance': []
            }
            classes.append(current_class)
            
            # Parse inheritance
            if inheritance:
                # Split by comma and clean up
                base_classes = [b.strip() for b in inheritance.split(',')]
                for base in base_classes:
                    # Remove access specifiers and virtual
                    base = re.sub(r'^(public|protected|private|virtual)\s+', '', base)
                    # Remove any trailing braces or other characters
                    base = re.sub(r'[{\s].*$', '', base)
                    if base:  # Only add if we have a valid base class name
                        current_class['inheritance'].append(base)
                        relationships.append({
                            'from': class_name,
                            'to': base,
                            'type': 'inheritance'
                        })
            continue
            
        # Look for method declarations
        if current_class:
            # Skip constructor declarations
            if re.match(r'\s*' + current_class['name'] + r'\s*\(', line):
                continue
                
            method_match = re.match(r'\s*(\w+(?:::\w+)*)\s+(\w+)\s*\([^)]*\)', line)
            if method_match:
                return_type, method_name = method_match.groups()
                # Skip main function
                if method_name == 'main':
                    continue
                current_class['methods'].append({
                    'name': method_name,
                    'return_type': return_type
                })
                continue
                
            # Look for attribute declarations
            attr_match = re.match(r'\s*(\w+(?:::\w+)*)\s+(\w+)\s*;', line)
            if attr_match:
                attr_type, attr_name = attr_match.groups()
                current_class['attributes'].append({
                    'name': attr_name,
                    'type': attr_type
                })
                
                # Check if attribute type is a class (potential composition/aggregation)
                if any(cls['name'] == attr_type for cls in classes):
                    relationships.append({
                        'from': current_class['name'],
                        'to': attr_type,
                        'type': 'composition'
                    })
    
    return classes, relationships

def generate_plantuml(classes, relationships):
    """Generate PlantUML code from class information."""
    plantuml = "@startuml\n"
    
    # Add classes
    for cls in classes:
        plantuml += f"class {cls['name']} {{\n"
        
        # Add attributes
        for attr in cls['attributes']:
            # Escape special characters in attribute names and types
            attr_type = attr['type'].replace('<', '\\<').replace('>', '\\>')
            attr_name = attr['name'].replace('<', '\\<').replace('>', '\\>')
            plantuml += f"  {attr_type} {attr_name}\n"
        
        # Add methods
        for method in cls['methods']:
            # Escape special characters in method names and return types
            return_type = method['return_type'].replace('<', '\\<').replace('>', '\\>')
            method_name = method['name'].replace('<', '\\<').replace('>', '\\>')
            plantuml += f"  {return_type} {method_name}()\n"
        
        plantuml += "}\n\n"
    
    # Add relationships
    for rel in relationships:
        if rel['type'] == 'inheritance':
            # Escape special characters in class names
            from_class = rel['from'].replace('<', '\\<').replace('>', '\\>')
            to_class = rel['to'].replace('<', '\\<').replace('>', '\\>')
            plantuml += f"{to_class} <|-- {from_class}\n"
        elif rel['type'] == 'composition':
            # Escape special characters in class names
            from_class = rel['from'].replace('<', '\\<').replace('>', '\\>')
            to_class = rel['to'].replace('<', '\\<').replace('>', '\\>')
            plantuml += f"{from_class} *-- {to_class}\n"
    
    plantuml += "@enduml"
    
    # Log the generated PlantUML code
    logger.debug(f"Generated PlantUML code:\n{plantuml}")
    
    return plantuml

def generate_diagram(input_file, output_dir, project_name):
    """Generate UML diagram using PlantUML."""
    try:
        # Read the input file
        with open(input_file, 'r') as f:
            content = f.read()
            logger.info(f"Input file content:\n{content}")
        
        # Extract class information and relationships
        classes, relationships = extract_class_info(content)
        
        # Log extracted information
        logger.info(f"Extracted classes: {classes}")
        logger.info(f"Extracted relationships: {relationships}")
        
        # Generate PlantUML code
        plantuml_code = generate_plantuml(classes, relationships)
        
        # Save PlantUML code to a temporary file
        plantuml_file = os.path.join(output_dir, f'{project_name}.puml')
        with open(plantuml_file, 'w', encoding='utf-8') as f:
            f.write(plantuml_code)
        
        # Log the file path and content
        logger.info(f"Saved PlantUML code to: {plantuml_file}")
        logger.info(f"PlantUML code:\n{plantuml_code}")
        
        # Verify the file was written correctly
        with open(plantuml_file, 'r', encoding='utf-8') as f:
            written_content = f.read()
            if written_content != plantuml_code:
                logger.error("PlantUML file content mismatch!")
                logger.error(f"Expected:\n{plantuml_code}")
                logger.error(f"Got:\n{written_content}")
                raise Exception("PlantUML file content mismatch")
        
        # Get the absolute path to the jar file
        jar_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'plantuml-mit-1.2025.2.jar')
        
        # Generate PNG using PlantUML
        cmd = [
            'java',
            '-jar', jar_path,
            '-tpng',
            '-charset', 'UTF-8',  # Specify UTF-8 encoding
            plantuml_file
        ]
        
        logger.info(f"Running command: {' '.join(cmd)}")
        result = subprocess.run(cmd, cwd=output_dir, capture_output=True, text=True)
        
        if result.returncode != 0:
            error_msg = f"PlantUML failed with error:\n{result.stderr}\nCommand output:\n{result.stdout}"
            logger.error(error_msg)
            
            # Log the contents of the PlantUML file
            with open(plantuml_file, 'r', encoding='utf-8') as f:
                logger.error(f"PlantUML file contents:\n{f.read()}")
            
            raise Exception(error_msg)
        
        return True
    except Exception as e:
        logger.exception("Error generating diagram")
        raise

def allowed_file(filename):
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in app.config['ALLOWED_EXTENSIONS']

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/upload', methods=['GET', 'POST'])
def upload_file():
    if request.method == 'GET':
        return render_template('upload.html')
    
    try:
        if 'file' not in request.files:
            logger.error("No file part in request")
            return jsonify({'error': 'No file part'}), 400
        
        file = request.files['file']
        if file.filename == '':
            logger.error("No selected file")
            return jsonify({'error': 'No selected file'}), 400
        
        if not allowed_file(file.filename):
            logger.error(f"Invalid file type: {file.filename}")
            return jsonify({'error': 'Invalid file type'}), 400
        
        filename = secure_filename(file.filename)
        logger.info(f"Processing file: {filename}")
        
        # Create a temporary directory
        temp_dir = tempfile.mkdtemp()
        file_path = os.path.join(temp_dir, filename)
        logger.info(f"Saving file to: {file_path}")
        
        file.save(file_path)
        
        try:
            # Generate UML diagram
            output_dir = app.config['UPLOAD_FOLDER']
            logger.info(f"Output directory: {output_dir}")
            
            # Ensure output directory exists
            os.makedirs(output_dir, exist_ok=True)
            
            # Generate diagram
            project_name = filename.split('.')[0]
            generate_diagram(file_path, output_dir, project_name)
            
            diagram_path = os.path.join(output_dir, f'{project_name}.png')
            logger.info(f"Looking for diagram at: {diagram_path}")
            
            if os.path.exists(diagram_path):
                logger.info("Diagram generated successfully")
                return jsonify({
                    'success': True,
                    'diagram_path': diagram_path,
                    'filename': filename
                })
            else:
                logger.error("Diagram file not found after generation")
                return jsonify({'error': 'Failed to generate diagram: Output file not found'}), 500
                
        except Exception as e:
            logger.exception("Error during diagram generation")
            return jsonify({'error': str(e)}), 500
        finally:
            # Clean up temporary directory
            try:
                shutil.rmtree(temp_dir)
            except Exception as e:
                logger.warning(f"Failed to clean up temp directory: {e}")
        
    except Exception as e:
        logger.exception("Unexpected error during file upload")
        return jsonify({'error': f'An unexpected error occurred: {str(e)}'}), 500

@app.route('/diagrams/<filename>')
def view_diagram(filename):
    try:
        diagram_name = filename.split('.')[0]
        diagram_path = os.path.join(
            app.config['UPLOAD_FOLDER'],
            f'{diagram_name}.png'
        )
        
        logger.info(f"Looking for diagram at: {diagram_path}")
        
        if not os.path.exists(diagram_path):
            logger.error(f"Diagram not found: {diagram_path}")
            return render_template('error.html', message='Diagram not found'), 404
        
        return render_template('view.html', 
                             diagram_path=diagram_path,
                             filename=filename)
    except Exception as e:
        logger.exception("Error viewing diagram")
        return render_template('error.html', message=str(e)), 500

@app.route('/diagrams/<filename>/download')
def download_diagram(filename):
    try:
        diagram_name = filename.split('.')[0]
        diagram_path = os.path.join(
            app.config['UPLOAD_FOLDER'],
            f'{diagram_name}.png'
        )
        
        if not os.path.exists(diagram_path):
            return render_template('error.html', message='Diagram not found'), 404
        
        return send_file(diagram_path, as_attachment=True)
    except Exception as e:
        logger.exception("Error downloading diagram")
        return render_template('error.html', message=str(e)), 500

if __name__ == '__main__':
    app.run(debug=True) 