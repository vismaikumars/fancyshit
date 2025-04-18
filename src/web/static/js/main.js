document.addEventListener('DOMContentLoaded', function() {
    // Handle drag and drop functionality
    const dropZone = document.querySelector('.drop-zone');
    const fileInput = document.querySelector('#file');
    const uploadForm = document.querySelector('#upload-form');
    const submitButton = document.querySelector('#submit-button');
    const loadingSpinner = document.querySelector('#loading-spinner');

    if (dropZone) {
        // Prevent default drag behaviors
        ['dragenter', 'dragover', 'dragleave', 'drop'].forEach(eventName => {
            dropZone.addEventListener(eventName, preventDefaults, false);
            document.body.addEventListener(eventName, preventDefaults, false);
        });

        // Highlight drop zone when dragging over it
        ['dragenter', 'dragover'].forEach(eventName => {
            dropZone.addEventListener(eventName, highlight, false);
        });

        ['dragleave', 'drop'].forEach(eventName => {
            dropZone.addEventListener(eventName, unhighlight, false);
        });

        // Handle dropped files
        dropZone.addEventListener('drop', handleDrop, false);

        // Handle file input change
        fileInput.addEventListener('change', handleFiles, false);
    }

    if (uploadForm) {
        uploadForm.addEventListener('submit', handleSubmit);
    }

    function preventDefaults(e) {
        e.preventDefault();
        e.stopPropagation();
    }

    function highlight(e) {
        dropZone.classList.add('highlight');
    }

    function unhighlight(e) {
        dropZone.classList.remove('highlight');
    }

    function handleDrop(e) {
        const dt = e.dataTransfer;
        const files = dt.files;
        fileInput.files = files;
        handleFiles({ target: { files: files } });
    }

    function handleFiles(e) {
        const files = e.target.files;
        const fileName = document.querySelector('#file-name');
        
        if (files.length > 0) {
            fileName.textContent = files[0].name;
            dropZone.classList.add('has-file');
        } else {
            fileName.textContent = 'No file chosen';
            dropZone.classList.remove('has-file');
        }
    }

    function handleSubmit(e) {
        if (submitButton && loadingSpinner) {
            submitButton.disabled = true;
            loadingSpinner.classList.remove('d-none');
        }
    }
}); 