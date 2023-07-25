#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <condition_variable>

using namespace std::chrono_literals;

class Clients
{
public:
	Clients(int max_clients)
	{
		_max_clients = max_clients;
		_clients_counter = 0;
		_clients_queue = 0;
	};
	void add_client(std::memory_order memory_flag_store)
	{
		while (_clients_counter < _max_clients)
		{
			
			++_clients_counter;
			std::cout << "\x1B[31mClients: Client number " << _clients_counter << " added.\n\x1B[0m";
			_clients_queue.fetch_add(1, memory_flag_store);
			std::this_thread::sleep_for(1s);
		}
	}
	friend class Teller;
private:
	int _max_clients;
	int _clients_counter;
	static std::atomic<int> _clients_queue;
};

std::atomic<int> Clients::_clients_queue = 0;

class Teller
{
public:
	Teller() = default;
	void take_clients(std::memory_order memory_flag_store, std::memory_order memory_flag_load)
	{

		do//ожидание первого клиента
		{
			std::this_thread::yield();
		} while (Clients::_clients_queue.load(memory_flag_load) == 0);
		
		while (Clients::_clients_queue.load(memory_flag_load) > 0)
		{
			++_clients_counter;
			std::cout << "\x1B[32mTeller: Client number " << _clients_counter << " is taken.\n\x1B[0m";
			
			Clients::_clients_queue.fetch_sub(1, memory_flag_load);
			std::this_thread::sleep_for(2s);			
		}
	}
private:
	int _clients_counter = 0;
};


int main()
{
	const int NUM_CLIENTS = 6;

	std::cout << "\x1B[36m" << "std::memory_order_seq_cst" << "\x1B[0m" << std::endl;
	{
		Clients clients(NUM_CLIENTS);
		Teller teller1;
		Teller teller2;
		std::memory_order memory_flag_store = std::memory_order_seq_cst;
		std::memory_order memory_flag_load = std::memory_order_seq_cst;		
		std::thread t1(&Clients::add_client, clients, memory_flag_store);
		std::thread t2(&Teller::take_clients, teller1, memory_flag_store, memory_flag_load);
		if (t1.joinable())
		{
			t1.join();
		}
		if (t2.joinable())
		{
			t2.join();
		}
	}
	std::cout << "\x1B[36m" << "std::memory_order_relaxed" << "\x1B[0m" << std::endl;
	{
		Clients clients(NUM_CLIENTS);
		Teller teller1;
		Teller teller2;
		std::memory_order memory_flag_store = std::memory_order_relaxed;
		std::memory_order memory_flag_load = std::memory_order_relaxed;
		std::thread t1(&Clients::add_client, clients, memory_flag_store);
		std::thread t2(&Teller::take_clients, teller1, memory_flag_store, memory_flag_load);
		if (t1.joinable())
		{
			t1.join();
		}
		if (t2.joinable())
		{
			t2.join();
		}
	}
	std::cout << "\x1B[36m" << "std::memory_order_release + std::memory_order_acquire" << "\x1B[0m" << std::endl;
	{
		Clients clients(NUM_CLIENTS);
		Teller teller1;
		Teller teller2;
		std::memory_order memory_flag_store = std::memory_order_release;
		std::memory_order memory_flag_load = std::memory_order_acquire;
		std::thread t1(&Clients::add_client, clients, memory_flag_store);
		std::thread t2(&Teller::take_clients, teller1, memory_flag_store, memory_flag_load);
		if (t1.joinable())
		{
			t1.join();
		}
		if (t2.joinable())
		{
			t2.join();
		}
	}
	std::cout << "\x1B[36m" << "std::memory_order_acq_rel + std::memory_order_acquire" << "\x1B[0m" << std::endl;
	{
		Clients clients(NUM_CLIENTS);
		Teller teller1;
		Teller teller2;
		std::memory_order memory_flag_store = std::memory_order_acq_rel;
		std::memory_order memory_flag_load = std::memory_order_acquire;
		std::thread t1(&Clients::add_client, clients, memory_flag_store);
		std::thread t2(&Teller::take_clients, teller1, memory_flag_store, memory_flag_load);
		if (t1.joinable())
		{
			t1.join();
		}
		if (t2.joinable())
		{
			t2.join();
		}
	}
}


