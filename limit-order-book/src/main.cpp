#include "LimitOrderBook.h"
#include "Order.h"
#include <iostream>

int main() {
    LimitOrderBook book;

    std::cout << std::endl << "Adding initial resting orders" << std::endl;
    book.add_order({Side::SELL, 100, 102}); // Ask 100 @ 102
    book.add_order({Side::SELL, 50,  101}); // Ask 50 @ 101
    book.add_order({Side::BUY,  40,  99});  // Bid 40 @ 99
    book.add_order({Side::BUY,  25,  98});  // Bid 25 @ 98
    book.print_book();

    Order order = {Side::BUY, 10, 200};
    std::cout << std:: endl << "Partial fill: " << order << std::endl;
    book.add_order(order);
    book.print_book();

    order = {Side::BUY, 150, 150};
    std::cout << std:: endl << "Removing liquidity: " << order << std::endl;
    book.add_order(order);    
    book.print_book();

    order = {Side::SELL, 10, 150};
    std::cout << std:: endl << "Matches and full fills: " << order << std::endl;
    book.add_order(order);    
    book.print_book();

    return 0;
}