#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Postgres.h>
#include <memory>
#include <string>
#include<Windows.h>

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

int main() {
    //setlocale(LC_ALL, "RU");
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
        std::unique_ptr<Wt::Dbo::backend::Postgres>connection{new Wt::Dbo::backend::Postgres(connectionString) };

        session.setConnection(move(connection));

        // Создание таблиц
        session.mapClass<Publisher>("publisher");
        session.mapClass<Book>("book");
        session.mapClass<Shop>("shop");
        session.mapClass<Stock>("stock");
        session.mapClass<Sale>("sale");

        dbo::Transaction transaction(session);
        session.createTables();

        std::cout << "Таблицы успешно созданы!" << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}