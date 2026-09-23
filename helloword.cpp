#include <iostream>

class shape{
    public:
    double area;
    double volume;

};
class Cube : public shape{
    public:
        double side;
    Cube(double side){
        this -> side = side;
        this -> area = side * side * 6;
        this -> volume = side * side * side;
    }
};
class sphere : public shape{
    public:
        double radius;
    sphere(double radius){
        this -> radius = radius;
        this -> area = 4 * 3.14159 * (radius* radius);
        this -> volume = (4/3.0) * 3.14159 * (radius * radius *radius);
    }
};

int main()
{
    Cube cube(15);
    sphere sphere(10);


    std::cout << "area: "<< cube.area << "cm\n";
    std::cout << "volume: "<< cube.volume << "cm\n";

    std::cout << "area: "<< sphere.area << "cm\n";
    std::cout << "volume: "<< sphere.volume << "cm\n";

    return 0;
}

