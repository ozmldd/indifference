#include <iostream>

int main()
{

    std::string question [] = {"1. What year was C++ created?: ",
                                "2. Who invented C++?: ",
                                "3. What was the predecessor of C++: ",
                                "4. What measurement system is widely used around the globe?: "};

    std::string options [][4] = {{"A. 1969", "B. 1975", "C. 1985", "D. 1989"},
                                {"A. Guido van Rossum", "B. Bjane Stroustrup", "C. John Carmack", "D. Mark Zuckerberg"},
                                {"A. C", "B. C+", "C. C--","D. C#"},
                                {"A. Fahrenheit", "B. kelvin", "C. Rankine", "D. Celsius"}};


    char answerKey [4] ={'C', 'B', 'A', 'D'};

    int size = sizeof(question)/ sizeof(question[0]);
    char guess;
    int score = 0;

    for(int i = 0; i < size; i++)
    {
        std::cout << "*************** \n";
        std::cout << question[i] << '\n';
        std::cout << "***************\n";

        for(int j = 0; j < sizeof(options[i])/sizeof(options[i][0]); j++)
        {
            std::cout<<options[i][j] << '\n';
        }
        std::cin >> guess;
        guess = toupper(guess);

        if(guess == answerKey[i])
        {
            std::cout<<"Correct!\n";
            score++;
        }
        else
        {
            std::cout<<"Wrong!\n";
            std::cout<<"Answer: " << answerKey[i] << '\n';
        }

    }
    std::cout<<"****************************\n";
    std::cout<<"            Results         \n";
    std::cout<<"****************************\n";
    std::cout<<"Correct guess: "<< score << '\n';
    std::cout<<"# of questions: " << size << '\n';
    std::cout<< "score: "<< (score/(double)size) *100 << "% ";
    return 0;
}