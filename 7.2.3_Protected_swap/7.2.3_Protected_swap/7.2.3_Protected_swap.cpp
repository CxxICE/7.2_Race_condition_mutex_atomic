#include <iostream>
#include <mutex>
#include <algorithm>


class Data
{
public:
	Data(int x, int y, int z)
	{
		_x = x;
		_y = y;
		_z = z;
	}
	void print()
	{
		std::cout << "x = " << _x << "; y = " << _y << "; z = " << _z << std::endl;
	}
	std::mutex &get_mutex()
	{
		return m;
	}

	//с использованием std::lock
	static void swap1(Data &A, Data &B)
	{
		std::lock(A.get_mutex(), B.get_mutex());
		std::lock_guard<std::mutex> la(A.get_mutex(), std::adopt_lock);
		std::lock_guard<std::mutex> lb(B.get_mutex(), std::adopt_lock);
		
		std::swap(A._x, B._x);
		std::swap(A._y, B._y);
		std::swap(A._z, B._z);
	}

	//с использованием std::scoped_lock
	static void swap2(Data &A, Data &B)
	{
		std::scoped_lock sl(A.get_mutex(), B.get_mutex());

		std::swap(A._x, B._x);
		std::swap(A._y, B._y);
		std::swap(A._z, B._z);
	}

	//с использованием std::unique_lock
	static void swap3(Data &A, Data &B)
	{
		std::unique_lock la(A.get_mutex(), std::defer_lock);
		std::unique_lock lb(B.get_mutex(), std::defer_lock);
		std::lock(la, lb);

		std::swap(A._x, B._x);
		std::swap(A._y, B._y);
		std::swap(A._z, B._z);
	}
	
private:
	int _x, _y, _z;
	std::mutex m;
};




int main()
{
	setlocale(LC_ALL, "RU");

	{
		std::cout << "\x1B[36m" << "swap1 с использованием std::lock" << "\x1B[0m" << std::endl;
		Data d1(1, 2, 3);
		Data d2(4, 5, 6);
		std::cout << "d1: ";
		d1.print();
		std::cout << "d2: ";
		d2.print();
		
		//запустим три потока для проверки корректности работы функции и mutex
		std::thread t1(Data::swap1, std::ref(d1), std::ref(d2));
		std::thread t2(Data::swap1, std::ref(d1), std::ref(d2));
		std::thread t3(Data::swap1, std::ref(d1), std::ref(d2));
		if (t1.joinable()) { t1.join(); }
		if (t2.joinable()) { t2.join(); }
		if (t3.joinable()) { t3.join(); }

		std::cout << "swap1(d1, d2)" << std::endl;
		std::cout << "d1: ";
		d1.print();
		std::cout << "d2: ";
		d2.print();
	}

	{
		std::cout << "\x1B[36m" << "swap2 с использованием std::scoped_lock" << "\x1B[0m" << std::endl;
		Data d1(1, 2, 3);
		Data d2(4, 5, 6);
		std::cout << "d1: ";
		d1.print();
		std::cout << "d2: ";
		d2.print();

		//запустим три потока для проверки корректности работы функции и mutex
		std::thread t1(Data::swap2, std::ref(d1), std::ref(d2));
		std::thread t2(Data::swap2, std::ref(d1), std::ref(d2));
		std::thread t3(Data::swap2, std::ref(d1), std::ref(d2));
		if (t1.joinable()) { t1.join(); }
		if (t2.joinable()) { t2.join(); }
		if (t3.joinable()) { t3.join(); }

		std::cout << "swap2(d1, d2)" << std::endl;
		std::cout << "d1: ";
		d1.print();
		std::cout << "d2: ";
		d2.print();
	}

	{
		std::cout << "\x1B[36m" << "swap3 с использованием std::unique_lock" << "\x1B[0m" << std::endl;
		Data d1(1, 2, 3);
		Data d2(4, 5, 6);
		std::cout << "d1: ";
		d1.print();
		std::cout << "d2: ";
		d2.print();

		//запустим три потока для проверки корректности работы функции и mutex
		std::thread t1(Data::swap3, std::ref(d1), std::ref(d2));
		std::thread t2(Data::swap3, std::ref(d1), std::ref(d2));
		std::thread t3(Data::swap3, std::ref(d1), std::ref(d2));
		if (t1.joinable()) { t1.join(); }
		if (t2.joinable()) { t2.join(); }
		if (t3.joinable()) { t3.join(); }

		std::cout << "swap3(d1, d2)" << std::endl;
		std::cout << "d1: ";
		d1.print();
		std::cout << "d2: ";
		d2.print();
	}
	   
}