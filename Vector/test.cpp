#include <iostream>
#include "Vector.h"
#include <algorithm>
#include <vector>
#include <string>

using namespace miv;

/*
 * File: test.cpp
 * Author: Aleksandr
 * Created on April 21, 2023, 3:11 AM
 * Last update: 26.06.2025
 */

struct Point {
	double x, y;
	Point(double x_, double y_) : x(x_), y(y_) {}
	friend std::ostream& operator<<(std::ostream& os, Point const& p) {
		return os << "(" << p.x << ", " << p.y << ")";
	}
};

auto main() -> std::uint32_t {
	// В качестве проверки работоспособности, используем генерацию последовательности квадратов + point;
	Vector<int> squares;
	squares.reserve(10);
	for (int i = 1; i <= 10; ++i)
		squares.push_back(i * i);

	std::cout << "Squares: ";
	for (auto v : squares) std::cout << v << ' ';
	std::cout << "\n\n";

	// assign() сброс + заполнение
	squares.assign(5, -1);
	std::cout << "After assign(5, -1): ";
	for (auto v : squares) std::cout << v << ' ';
	std::cout << "\n\n";

	//insert() в середине вставляем {100, 200, 300}
	Vector<int> chunk{ 100, 200, 300 };
	squares.insert(squares.begin() + 2,
		chunk.begin(), chunk.end());
	std::cout << "After inserting {100,200,300} at pos=2: ";
	for (auto v : squares) std::cout << v << ' ';
	std::cout << "\n\n";

	// erase(range) удаляем первые 2 элемента
	squares.erase(squares.begin(), squares.begin() + 2);
	std::cout << "After erase[first,first+2): ";
	for (auto v : squares) std::cout << v << ' ';
	std::cout << "\n\n";

	// emplace() конструируем объекты Point
	Vector<Point> poly;
	poly.emplace_back(0.0, 1.0);
	poly.emplace_back(1.0, 2.5);
	poly.emplace_back(2.0, 4.0);
	std::cout << "Polynomial points: ";
	for (auto const& p : poly) std::cout << p << ' ';
	std::cout << "\n\n";

	// reverse iteration
	std::cout << "Points in reverse: ";
	for (auto it = poly.rbegin(); it != poly.rend(); ++it)
		std::cout << *it << ' ';
	std::cout << "\n\n";

	// comparison и swap
	Vector<std::string> a{ "one","two","three" };
	Vector<std::string> b{ "ONE","TWO","THREE" };
	std::cout << "a == b? " << (a == b ? "yes" : "no") << "\n";
	a.swap(b);
	std::cout << "After swap, a: ";
	for (auto const& s : a) std::cout << s << ' ';
	std::cout << "\n\n";

	// max_size и get_allocator
	std::cout << "Max size of squares: " << squares.max_size() << "\n";
	auto alloc = squares.get_allocator(); (void)alloc;

	return 0;
}