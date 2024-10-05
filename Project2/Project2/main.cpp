#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Postgres.h>
#include <memory>
#include <string>
#include <iostream>
#include <Windows.h>

#pragma execution_character_set("utf-8")

namespace dbo = Wt::Dbo;

// Класс Publisher (Издатель)
class Publisher {
public:
    std::string name;

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, name, "name");
        dbo::hasMany(a, books_, dbo::ManyToOne, "publisher");
    }

    typedef dbo::collection< dbo::ptr<class Book> > Books;
    Books books_;
};

// Класс Book (Книга)
class Book {
public:
    std::string title;
    dbo::ptr<Publisher> publisher;

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, title, "title");
        dbo::belongsTo(a, publisher, "publisher");
        dbo::hasMany(a, stocks_, dbo::ManyToOne, "book");
    }

    typedef dbo::collection< dbo::ptr<class Stock> > Stocks;
    Stocks stocks_;
};

// Класс Shop (Магазин)
class Shop {
public:
    std::string name;

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, name, "name");
        dbo::hasMany(a, stocks_, dbo::ManyToOne, "shop");
    }

    typedef dbo::collection< dbo::ptr<class Stock> > Stocks;
    Stocks stocks_;
};

// Класс Stock (Наличие на складе)
class Stock {
public:
    int count;
    dbo::ptr<Book> book;
    dbo::ptr<Shop> shop;

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, count, "count");
        dbo::belongsTo(a, book, "book");
        dbo::belongsTo(a, shop, "shop");
        dbo::hasMany(a, sales_, dbo::ManyToOne, "stock");
    }

    typedef dbo::collection< dbo::ptr<class Sale> > Sales;
    Sales sales_;
};

// Класс Sale (Продажа)
class Sale {
public:
    double price;
    std::string date_sale;
    int count;
    dbo::ptr<Stock> stock;

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, price, "price");
        dbo::field(a, date_sale, "date_sale");
        dbo::field(a, count, "count");
        dbo::belongsTo(a, stock, "stock");
    }
};

// Функция для вставки тестовых данных
void insertTestData(dbo::Session& session) {
    dbo::Transaction transaction(session);

    // Вставка издателей
    dbo::ptr<Publisher> publisherA = session.add(std::make_unique<Publisher>());
    publisherA.modify()->name = "Publisher A";

    dbo::ptr<Publisher> publisherB = session.add(std::make_unique<Publisher>());
    publisherB.modify()->name = "Publisher B";

    // Вставка книг
    dbo::ptr<Book> book1 = session.add(std::make_unique<Book>());
    book1.modify()->title = "Book 1";
    book1.modify()->publisher = publisherA;

    dbo::ptr<Book> book2 = session.add(std::make_unique<Book>());
    book2.modify()->title = "Book 2";
    book2.modify()->publisher = publisherB;

    // Вставка магазинов
    dbo::ptr<Shop> shopA = session.add(std::make_unique<Shop>());
    shopA.modify()->name = "Shop A";

    dbo::ptr<Shop> shopB = session.add(std::make_unique<Shop>());
    shopB.modify()->name = "Shop B";

    // Вставка наличия книг в магазинах
    dbo::ptr<Stock> stock1 = session.add(std::make_unique<Stock>());
    stock1.modify()->book = book1;
    stock1.modify()->shop = shopA;
    stock1.modify()->count = 10;

    dbo::ptr<Stock> stock2 = session.add(std::make_unique<Stock>());
    stock2.modify()->book = book2;
    stock2.modify()->shop = shopB;
    stock2.modify()->count = 5;

    transaction.commit();
}

// Функция для вывода магазинов, продающих книги определённого издателя
void printShopsSellingPublisherBooks(dbo::Session& session, const std::string& publisherName) {
    dbo::Transaction transaction(session);

    typedef dbo::collection<dbo::ptr<Shop>> ShopCollection;

    dbo::ptr<Publisher> publisher = session.find<Publisher>().where("name = ?").bind(publisherName);

    if (!publisher) {
        std::cout << "Издатель с именем \"" << publisherName << "\" не найден." << std::endl;
        return;
    }

    std::cout << "Магазины, продающие книги издателя \"" << publisherName << "\":" << std::endl;

    for (const dbo::ptr<Book>& book : publisher->books_) {
        for (const dbo::ptr<Stock>& stock : book->stocks_) {
            std::cout << "- " << stock->shop->name << std::endl;
        }
    }

    transaction.commit();
}

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 1000);

    try {
        // Подключение к базе данных PostgreSQL
        std::string connectionString =
            "host=localhost "
            "port=5432 "
            "user=postgres "
            "password=extreme190287 "
            "dbname=postgres ";

        dbo::Session session;
        std::unique_ptr<Wt::Dbo::backend::Postgres> connection{ new Wt::Dbo::backend::Postgres(connectionString) };
        session.setConnection(std::move(connection));

        // Создание таблиц
        session.mapClass<Publisher>("publisher");
        session.mapClass<Book>("book");
        session.mapClass<Shop>("shop");
        session.mapClass<Stock>("stock");
        session.mapClass<Sale>("sale");

        dbo::Transaction transaction(session);
        session.createTables();

        std::cout << "Таблицы успешно созданы!" << std::endl;

        // Вставка тестовых данных
        insertTestData(session);
        std::cout << "Тестовые данные успешно добавлены!" << std::endl;

        // Получение имени издателя от пользователя
        std::string publisherName;
        std::cout << "Введите имя издателя: ";
        std::getline(std::cin, publisherName);

        // Вывод магазинов, продающих книги данного издателя
        printShopsSellingPublisherBooks(session, publisherName);

    }
    catch (std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}