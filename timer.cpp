//
// Created by borisshapa on 28.12.2019.
//

#include "timer.h"

#include <cassert>
#include <iostream>

void timer::add(timer_element *time_elem) {
    priority_queue.insert(value_t(time_elem->wakeup, time_elem));
}

void timer::remove(timer_element *time_elem) {
    auto i = priority_queue.find(value_t(time_elem->wakeup, time_elem));
    priority_queue.erase(i);
}

bool timer::empty() const {
    return priority_queue.empty();
}

timer::clock_t::time_point timer::top() const {
    return priority_queue.begin()->first;
}

void timer::notify(clock_t::time_point now) {
    while (true) {
        if (priority_queue.empty()) {
            break;
        }

        auto i = priority_queue.begin();
        if (i->first > now) {
            break;
        }

        i->second->t = nullptr;
        i->second->callback();
        priority_queue.erase(i);
    }
}

timer_element::timer_element()
        : t(nullptr) {}

timer_element::timer_element(timer_element::callback_t callback)
        : t(nullptr), callback(std::move(callback)) {}

timer_element::timer_element(timer &t, clock_t::duration interval, callback_t callback)
        : t(&t), wakeup(clock_t::now() + interval), callback(std::move(callback)) {
    t.add(this);
}

timer_element::timer_element(timer &t, clock_t::time_point wakeup, callback_t callback)
        : t(&t), wakeup(wakeup), callback(std::move(callback)) {
    t.add(this);
}

timer_element::~timer_element() {
    if (t) {
        t->remove(this);
    }
}

void timer_element::restart(timer &timer, clock_t::duration interval) {
    if (this->t) {
        this->t->remove(this);
    }
    this->t = &timer;
    this->wakeup = clock_t::now() + interval;
    this->t->add(this);
}

