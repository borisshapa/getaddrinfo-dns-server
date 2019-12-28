//
// Created by borisshapa on 28.12.2019.
//

#include "timer.h"

#include <cassert>
#include <iostream>

void timer::add(timer_element *e) {
    queue.insert(value_t(e->wakeup, e));
}

void timer::remove(timer_element *e) {
    auto i = queue.find(value_t(e->wakeup, e));
    queue.erase(i);
}

bool timer::empty() const {
    return queue.empty();
}

timer::clock_t::time_point timer::top() const {
    return queue.begin()->first;
}

void timer::notify(clock_t::time_point now) {
    while (true) {
        if (queue.empty()) {
            break;
        }

        auto i = queue.begin();
        if (i->first > now) {
            break;
        }

        i->second->t = nullptr;
        i->second->callback();
        queue.erase(i);
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

void timer_element::set_callback(timer_element::callback_t callback_) {
    this->callback = std::move(callback_);
}

void timer_element::restart(timer &timer, clock_t::duration interval) {
    if (this->t) {
        this->t->remove(this);
    }
    this->t = &timer;
    this->wakeup = clock_t::now() + interval;
    this->t->add(this);
}

void timer_element::restart(timer &timer, clock_t::time_point wakeup_) {
    this->t->remove(this);
    this->t = &timer;
    this->wakeup = wakeup_;
    this->t->add(this);
}