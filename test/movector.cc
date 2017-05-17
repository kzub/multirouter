#include <iostream>

struct A {
  int value = 0;
  void set(int v) {
    value = v;
  }
  void print() {
    std::cout << "value: " << std::to_string(value) << std::endl;
  }

  A() {
    std::cout << "C" << std::endl;
  }

  A(const A& a) {
    std::cout << "CC" << std::endl;
  }

  A(const A&& a) {
    std::cout << "MC" << std::endl;
  }

  A& operator=(const A& a) {
    std::cout << "AC" << std::endl;
    value = a.value;
    return *this;
  }
};

void test(A&& a) {
  a.set(12);
}

int main() {
  A a;
  a.set(10);
  A b;
  b.set(20);

  test(std::move(a));
  a = b;
  a.print();
  return 0;
}