import  <functional>;
import  <iostream>;
import  <string>;

import basekit;


using namespace std;

void print(int a, double b, const char *c, std::string d) {
    std::cout << a << b << c << d << std::endl;
}

void test() {
    cout << "hellp" << std::endl;
}

int main(int argc, char const *argv[]) {
    using namespace basekit;
    ThreadPool poll;
    function func = []() {
        println("bonjour world");
    };
    poll.add(func);
    func = []() {
        println("guten morgen");
    };

    poll.add(func);

    return 0;
}
