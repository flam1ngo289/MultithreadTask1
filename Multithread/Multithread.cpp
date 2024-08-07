#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <Windows.h>

std::mutex mtx;
std::condition_variable cv;
int client_counter = 0;
bool finished = false;

void client(int max_clients) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::unique_lock<std::mutex> lock(mtx);
        if (client_counter < max_clients) {
            client_counter++;
            std::cout << "Новый клиент добавлен. Клиентов в очереди: " << client_counter << std::endl;
        }
        cv.notify_all();
        if (finished) break;
    }
}

void operator_thread() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return client_counter > 0 || finished; });
        if (client_counter > 0) {
            client_counter--;
            std::cout << "Клиент обслужен. Клиентов в очереди: " << client_counter << std::endl;
        }
        else if (finished) {
            break;
        }
    }
}

int main() {
    setlocale(LC_ALL, "RU");

    int max_clients;
    std::cout << "Введите максимальное количество клиентов: ";
    std::cin >> max_clients;

    std::thread client_thread(client, max_clients);
    std::thread operator_thread_instance(operator_thread);

    std::this_thread::sleep_for(std::chrono::seconds(max_clients * 2 + 5)); 
    {
        std::lock_guard<std::mutex> lock(mtx);
        finished = true;
    }
    cv.notify_all();

    client_thread.join();
    operator_thread_instance.join();

    std::cout << "Все клиенты обслужены." << std::endl;
    return 0;
}
