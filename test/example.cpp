#include <string>
#include <vector>
#include <memory>

// Abstract base class
class Animal {
public:
    virtual ~Animal() = default;
    virtual void makeSound() const = 0;
    virtual std::string getName() const = 0;
};

// Concrete class
class Dog : public Animal {
private:
    std::string name;
    int age;
    
public:
    Dog(const std::string& name, int age) : name(name), age(age) {}
    
    void makeSound() const override {
        std::cout << "Woof!" << std::endl;
    }
    
    std::string getName() const override {
        return name;
    }
    
    int getAge() const {
        return age;
    }
};

// Template class
template<typename T>
class Container {
private:
    std::vector<T> items;
    
public:
    void add(const T& item) {
        items.push_back(item);
    }
    
    T get(size_t index) const {
        return items[index];
    }
    
    size_t size() const {
        return items.size();
    }
};

// Singleton pattern
class Logger {
private:
    static Logger* instance;
    Logger() = default;
    
public:
    static Logger* getInstance() {
        if (!instance) {
            instance = new Logger();
        }
        return instance;
    }
    
    void log(const std::string& message) {
        std::cout << "LOG: " << message << std::endl;
    }
};

// Factory pattern
class Shape {
public:
    virtual ~Shape() = default;
    virtual double getArea() const = 0;
};

class Circle : public Shape {
private:
    double radius;
    
public:
    Circle(double radius) : radius(radius) {}
    
    double getArea() const override {
        return 3.14159 * radius * radius;
    }
};

class ShapeFactory {
public:
    static std::unique_ptr<Shape> createCircle(double radius) {
        return std::make_unique<Circle>(radius);
    }
};

// Observer pattern
class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(const std::string& message) = 0;
};

class Subject {
private:
    std::vector<Observer*> observers;
    
public:
    void attach(Observer* observer) {
        observers.push_back(observer);
    }
    
    void detach(Observer* observer) {
        observers.erase(
            std::remove(observers.begin(), observers.end(), observer),
            observers.end()
        );
    }
    
    void notify(const std::string& message) {
        for (auto observer : observers) {
            observer->update(message);
        }
    }
}; 