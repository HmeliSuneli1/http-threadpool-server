# http-threadpool-server


### Сборка и запуск

Сборка в Release-режиме:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
```

Запуск сервера:

```bash
./build/server --port 8080 --threads 8
```

Сборка с санитайзерами (TSan / ASan):

```bash
cmake -B build-tsan -DCMAKE_CXX_FLAGS="-fsanitize=thread -g" && cmake --build build-tsan -j
cmake -B build-asan -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -g" && cmake --build build-asan -j

```

Запуск unit-тестов:
```bash
ctest --test-dir build --output-on-failure
```

### Распределение ролей и зон отвественности
| Участник | Роль и зона ответственности |
| :--- | :--- |
| **Кононов Дмитрий** | Сетевой слой (POSIX sockets, Acceptor, Keep-Alive), `ThreadPool`, Graceful Shutdown (`SIGINT`/`SIGTERM`), CI и нагрузочное тестирование |
| **Железникова Ольга** | Парсер и сериализатор HTTP/1.1, `Router` (8 REST-ручек), потокобезопасное хранилище `ItemStorage` (Optimistic Locking, шард-мьютексы), Unit-тесты |
