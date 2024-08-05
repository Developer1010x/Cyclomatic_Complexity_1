#include <iostream>
#include <type_traits>

// gcd function
int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

// isPrime function
bool isPrime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

// complexOperation function
int complexOperation(int x) {
    if (x > 10) {
        if (x < 20) {
            return x * 2;
        } else if (x < 30) {
            return x * 3;
        } else {
            return x * 4;
        }
    }
    return x;
}

// moreComplexOperation function
void moreComplexOperation(int y) {
    switch (y) {
        case 1:
            std::cout << "One";
            break;
        case 2:
            std::cout << "Two";
            break;
        case 3:
            std::cout << "Three";
            break;
        case 4:
            std::cout << "Four";
            break;
        case 5:
            std::cout << "Five";
            break;
        default:
            std::cout << "Other";
            break;
    }
    if (y % 2 == 0) {
        if (y > 10) {
            std::cout << " Even and greater than 10";
        } else {
            std::cout << " Even and less than or equal to 10";
        }
    } else {
        if (y < 5) {
            std::cout << " Odd and less than 5";
        } else {
            std::cout << " Odd and greater than or equal to 5";
        }
    }
}

// main function
int main() {
    int a = 30, b = 20;
    std::cout << "GCD of " << a << " and " << b << " is " << gcd(a, b) << std::endl;

    int num = 29;
    std::cout << num << " is " << (isPrime(num) ? "prime" : "not prime") << std::endl;

    int x = 25;
    std::cout << "Complex operation result: " << complexOperation(x) << std::endl;

    int y = 3;
    moreComplexOperation(y);
    std::cout << std::endl;

    return 0;
}
