# BC_model
Данный проект это моя дипломная работа, задача была написать приложение моделирующее работу блокчейна.

В проекте использовались: фрэймворк Qt, криптографическая билеотека OpenSSL, СУБД SQLite

Данное приложение моделирует работу блокчейн сети. Позволяет добавлять узлы, проводить транзакции, задавать сложность вычисляемого хэша у блоков. 
Алгоритм консенсуса используемый в сети: Proof-of-Work

Узлы будут отвечают за основной функционал, т.е. работу с
транзакциями, блоками и базой данных. Каждый узел имеет свою базу
данных для демонстрации того, что сеть приходит к консенсусу. 
При проверке, базы должны быть одинаковыми.

Имитировать децентрализованную распределенную систему будут узлы
и ядро программы, узлы будут осуществлять связь между собой с помощью
ядра, ядро в данном случае можно представить, как среду передачи данных между узлами сети.

<img src="https://github.com/Lmydr/diplomBC/blob/main/pic/%D0%A1%D1%85%D0%B5%D0%BC%D0%B0%20%D0%BF%D1%80%D0%BE%D0%B3%D1%80%D0%B0%D0%BC%D0%BC%D1%8B.png" width="700">

Управление осуществляется с помощью графического интерфейса.

<img src="https://github.com/Lmydr/diplomBC/blob/main/pic/%D0%93%D0%BB%D0%B0%D0%B2%D0%BD%D0%BE%D0%B5%20%D0%BE%D0%BA%D0%BD%D0%BE.png" width="700">

1. «Add Node» добавить узел – добавляет новый узел.
2. «Start» – при нажатии все добавленные узлы начинают
непрерывно высчитывать блоки.
3. «Stop» – останавливает вычисление блоков.
44
4. «Accept» – применяет заданную в (5) сложность хэша для блока.
5. Виджет для ввода сложности вычисляемого хэша блока, задается
числом от 2 до 8. Это число определяет количество нулей в начале для 16-
ричной записи хэша.
6. «Send Tokens» – открывает окно для осуществления транзакции.
7. «make block» – при нажатии все узлы начинают высчитывать хэш
блока, но только один раз. Это функция применяется для пошагового режима
работы.
8. Поле лога выбранного узла, в него выводится информация для
отслеживания текущей работы данного узла.
9. Вкладки для переключения между узлами сети.
10. Информация о выбранном в данный момент узле.

<img src="https://github.com/Lmydr/diplomBC/blob/main/pic/%D0%9E%D0%BA%D0%BD%D0%BE%20%D1%82%D1%80%D0%B0%D0%BD%D0%B7%D0%B0%D0%BA%D1%86%D0%B8%D0%B8.png" width="500">

1. Информация об отправителе.
2. Поля ввода адреса получателя.
3. Поле ввода количества передаваемых токенов.
4. Кнопка отправить.
5. Кнопка отмены

