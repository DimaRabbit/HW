#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Postgres.h>
#include <memory>
#include <string>
#include <iostream>
#include <Windows.h>

#pragma execution_character_set("utf-8")

namespace dbo = Wt::Dbo;

// ����� Publisher (��������)
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

// ����� Book (�����)
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

// ����� Shop (�������)
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

// ����� Stock (������� �� ������)
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

// ����� Sale (�������)
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

// ������� ��� ������� �������� ������
void insertTestData(dbo::Session& session) {
    dbo::Transaction transaction(session);

    // ������� ���������
    dbo::ptr<Publisher> publisherA = session.add(std::make_unique<Publisher>());
    publisherA.modify()->name = "Publisher A";

    dbo::ptr<Publisher> publisherB = session.add(std::make_unique<Publisher>());
    publisherB.modify()->name = "Publisher B";

    // ������� ����
    dbo::ptr<Book> book1 = session.add(std::make_unique<Book>());
    book1.modify()->title = "Book 1";
    book1.modify()->publisher = publisherA;

    dbo::ptr<Book> book2 = session.add(std::make_unique<Book>());
    book2.modify()->title = "Book 2";
    book2.modify()->publisher = publisherB;

    // ������� ���������
    dbo::ptr<Shop> shopA = session.add(std::make_unique<Shop>());
    shopA.modify()->name = "Shop A";

    dbo::ptr<Shop> shopB = session.add(std::make_unique<Shop>());
    shopB.modify()->name = "Shop B";

    // ������� ������� ���� � ���������
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

// ������� ��� ������ ���������, ��������� ����� ������������ ��������
void printShopsSellingPublisherBooks(dbo::Session& session, const std::string& publisherName) {
    dbo::Transaction transaction(session);

    typedef dbo::collection<dbo::ptr<Shop>> ShopCollection;

    dbo::ptr<Publisher> publisher = session.find<Publisher>().where("name = ?").bind(publisherName);

    if (!publisher) {
        std::cout << "�������� � ������ \"" << publisherName << "\" �� ������." << std::endl;
        return;
    }

    std::cout << "��������, ��������� ����� �������� \"" << publisherName << "\":" << std::endl;

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
        // ����������� � ���� ������ PostgreSQL
        std::string connectionString =
            "host=localhost "
            "port=5432 "
            "user=postgres "
            "password=extreme190287 "
            "dbname=postgres ";

        dbo::Session session;
        std::unique_ptr<Wt::Dbo::backend::Postgres> connection{ new Wt::Dbo::backend::Postgres(connectionString) };
        session.setConnection(std::move(connection));

        // �������� ������
        session.mapClass<Publisher>("publisher");
        session.mapClass<Book>("book");
        session.mapClass<Shop>("shop");
        session.mapClass<Stock>("stock");
        session.mapClass<Sale>("sale");

        dbo::Transaction transaction(session);
        session.createTables();

        std::cout << "������� ������� �������!" << std::endl;

        // ������� �������� ������
        insertTestData(session);
        std::cout << "�������� ������ ������� ���������!" << std::endl;

        // ��������� ����� �������� �� ������������
        std::string publisherName;
        std::cout << "������� ��� ��������: ";
        std::getline(std::cin, publisherName);

        // ����� ���������, ��������� ����� ������� ��������
        printShopsSellingPublisherBooks(session, publisherName);

    }
    catch (std::exception& e) {
        std::cerr << "������: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}