#include <windows.h>

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <random>

using namespace std::chrono_literals;
using duration_ms = std::chrono::duration<double, std::milli>;

class Processor
{
public:
	Processor(const int count_threads, const duration_ms total_duration)
	{
		_count_threads = count_threads;
		_total_duration = total_duration;
		_VT.resize(count_threads);
		_VSP.resize(count_threads);
	}

	void calculate()//метод распараллеливает вычисления на несколько подпроцессов и дополнительно создает поток-наблюдатель для вывода на экран
	{		
		duration_ms sub_duration = _total_duration / _count_threads;
		
		for (int i = 0; i < _count_threads; ++i)
		{
			if (i < _count_threads - 1)
			{
				_VT[i] = std::thread([this, i, sub_duration]()
					{
						_VSP[i].calculate(sub_duration); 
					});
				//_VT[i] = std::thread(&Processor::SubProcessor::calculate, &(this->_VSP)[i], sub_duration);
			}
			else
			{
				_VT[i] = std::thread([this, i, sub_duration]()
					{
					_VSP[i].calculate(_total_duration - (_count_threads - 1) * sub_duration); //интервал последнего потока по остаточному принципу
					});
				//_VT[i] = std::thread(&Processor::SubProcessor::calculate, &(this->_VSP)[i], _total_duration - (_count_threads - 1) * sub_duration);
			}
		}
		//печать в cout в отдельном потоке
		std::thread t_print([this]() {print(); });

		//для единовременного запуска потоков
		_go = true;

		for (auto &vt : _VT)
		{
			if (vt.joinable()) { vt.join(); }
		}
		if (t_print.joinable()) { t_print.join(); }
	}

	
private:
	void print()
	{
		while (!Processor::_go)
		{
			std::this_thread::yield();
		}
		int end_calc = 0;
		while (true)
		{
			end_calc = 0;
			system("cls");
			int counter = 0;
			for (auto &el : _VSP)
			{					
				++counter;
				bool finish = el.get_finish()->load(std::memory_order_acquire);//для обеспечения гарантии, что все переменные в потоке SubProcess обновлены
				std::cout << "номер потока: " << counter << "\n"
					<< "id_потока: " << el.get_thread_id() << "\n"
					<< "процент: " << el.get_percent();
				if (finish == true)
				{
					++end_calc;
					std::cout << "; Суммарное время работы потока: " << el.get_delta().count() << " ms";
				}
				std::cout << "\n";
				for (int i = 0; i < el.get_percent(); ++i)
				{
					if (el.get_err(i))
					{
						std::cout << "\x1B[41m \x1B[0m";
					}
					else
					{
						std::cout << "\x1B[44m \x1B[0m"; 
					}					
				}
				std::cout << "\n";
			}
			if (end_calc == _count_threads)
			{
				break;
			}
			
		}
		std::cout << "\x1B[32mРасчет окончен нажмите Enter\n\x1B[0m";
		std::getchar();
	}

	class SubProcessor
	{
	public:
		SubProcessor()
		{
			++_proc_count;
			_proc_id = ++_id_generate;
			_delta = 0ms;
			_percent = 0;
			_thread_id = std::this_thread::get_id();
			_finish = false;
			std::fill_n(_error_arr, _PERCENTS_MAX + 1, false);
		};
		~SubProcessor()
		{
			--_proc_count;			
		};
		SubProcessor(const SubProcessor &other)
		{
			++_proc_count;
			_proc_id = ++_id_generate;
			_delta = other._delta;
			_percent = other._percent;
			_thread_id = std::this_thread::get_id();
			_finish.store(other._finish.load());
			for (int i = 0; i < _PERCENTS_MAX + 1; ++i)
			{
				_error_arr[i] = other._error_arr[i];
			}
		}
		
		void calculate(const duration_ms sub_duration)
		{
			while (!Processor::_go)
			{
				std::this_thread::yield();
			}
			auto start = std::chrono::high_resolution_clock::now();
			duration_ms cur_duration = 0ms;
			duration_ms percent_duration = sub_duration / 100;
			_thread_id = std::this_thread::get_id();						
			std::mt19937 gen;
			std::uniform_int_distribution<int> dist(1, 10);
			while (cur_duration < sub_duration)
			{	
				try
				{					
					int error = dist(gen);
					if (error == _proc_id % 10 + 1)//генерация рандомных ошибок в зависимости от id процесса
					{
						throw std::exception();
					}					
				}
				catch(std::exception)
				{
					_error_arr[_percent] = true;
					continue;//расчет для данного процента повторится в случае ошибки, не инкрементируем процент и длительность расчета
				}
				if (_percent < 99)
				{
					cur_duration += percent_duration;
				}
				else
				{
					cur_duration = sub_duration;//последний интервал расчета по остаточному принципу
				}
				++_percent;
				std::this_thread::sleep_for(percent_duration);//имитация расчета, каждый цикл = 1 %		
			}
			auto end = std::chrono::high_resolution_clock::now();
			_delta = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
			_finish.store(true, std::memory_order_release);//для обеспечения гарантии, что все переменные в потоке SubProcess обновлены и могут быть захвачены потоком c print
		};
		int get_id()
		{
			return _proc_id;
		};
		std::thread::id get_thread_id()
		{
			return _thread_id;
		};
		int get_percent()
		{
			return _percent;
		};
		duration_ms get_delta()
		{
			return _delta;
		};
		std::atomic<bool>* get_finish()
		{
			return &_finish;
		};
		bool get_err(int percent)
		{
			return _error_arr[percent];
		};
		
	private:
		static const int _PERCENTS_MAX = 100;
		static int _proc_count;				//подсчет кол-ва созданных объектов SubProcessor, декрементируется при разрушении объекта
		int _proc_id;						//номер потока/подрасчета
		duration_ms _delta;					//для фиксации итоговой длительности расчета
		int _percent;						//для фиксации выполненного процента расчета
		std::thread::id _thread_id;			//для фиксации id_потока
		std::atomic<bool> _finish;			//фиксация окончания вычисления, чтобы зафиксировать на последней итерации все переменные в корректном состоянии
		bool _error_arr[_PERCENTS_MAX + 1];	//массив фиксации озникающих ошибок вычислений
		static int _id_generate;			//для генерации уникальных id, при разрушении не декрементируется		
	};
	
	std::vector<std::thread> _VT;			
	std::vector<SubProcessor> _VSP;		
	int _count_threads;					//хранит требуемое кол-во потоков
	duration_ms _total_duration;		//хранит общую длительность расчета, которую требуется распределить на несколько потоков
	static std::atomic<bool> _go;
};

int Processor::SubProcessor::_proc_count = 0;
int Processor::SubProcessor::_id_generate = 0;

std::atomic<bool> Processor::_go = false;


int main()
{
	setlocale(LC_ALL, "RU");
	{
		//задаем сложность вычисления в виде длительности total_duration и распределяем длительность на кол-во потоков count_threads поровну в методе calculate объекта Processor
		const int count_threads = 5;
		const duration_ms total_duration = 50s;
		Processor proc(count_threads, total_duration);
		proc.calculate();
	}
	
	{
		const int count_threads = 7;
		const duration_ms total_duration = 50s;
		Processor proc(count_threads, total_duration);
		proc.calculate();
	}    
}


