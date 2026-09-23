#include <iostream>
#include <cmath>

int main ()
{

    double temp;
    char unit;

    std::cout<<"******Temperature conversion***************** \n";
    std::cout<<"F = fahrenheit\n";
    std::cout<<"C = celcius \n";
    std::cout<<"What unit would you like to convert to? ";
    std::cin>>unit;
    if (unit=='F' || unit =='f'){
        std::cout<<"Enter the temperature in celcius: ";
        std::cin>>temp;

        temp = (1.8*temp) + 32.00;
        std::cout<<"temperature is:   " << temp <<"F\n";
    }
    else if (unit == 'C'|| unit =='c'){
        std::cout<<"Enter the temperature in fahrenheit: ";
        std::cin>>temp;

        temp = (temp-32)/1.8;
        std::cout << "temperature is:   "<< temp<<"C\n";
    }
    else{
        std::cout<<"Please enter in only C or F\n";
    }

    std::cout<<"********************";


    return 0;
}