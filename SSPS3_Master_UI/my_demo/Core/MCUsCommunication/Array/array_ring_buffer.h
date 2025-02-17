#pragma once
#ifndef ARRAY_RING_BUFFER_H
#define ARRAY_RING_BUFFER_H

template<typename T, unsigned char Size>
struct ArrayRingBuffer
{
    volatile uint8_t head = 0;
    volatile uint8_t tail = 0;
    T buffer[Size];

    // Добавление элемента. Функция не выполняет переприсваивания динамической памяти.
    bool push(const T &item, bool place_in_front = false)
    {
        if (!place_in_front)
        {
            uint8_t nextTail = (tail + 1) % Size;

            if (nextTail == head)
            {
                // Буфер переполнен
                return false;
            }

            buffer[tail] = item;
            tail = nextTail;
            return true;
        }
        else
        {
            // вставка элемента в начало буфера
            uint8_t newHead = (head == 0) ? (Size - 1) : (head - 1);

            if (newHead == tail)
            {
                // Буфер переполнен
                return false;
            }

            head = newHead;
            buffer[head] = item;
            return true;
        }
    }

    // Извлечение элемента. True == сообщение назначено; False == буфер сообщений был пуст, сообщений нет
    bool pop(T &item)
    {
        if (head == tail) {
            // Буфер пуст
            return false;
        }
        item = buffer[head];
        head = (head + 1) % Size;
        return true;
    }

    // Количество элементов в буфере
    uint8_t count() const
    {
        if (tail >= head)
            return tail - head;
        else
            return Size - head + tail;
    }

    // Проверка, пуст ли буфер
    bool empty() const {
        return head == tail;
    }
};

#endif // !ARRAY_RING_BUFFER_H