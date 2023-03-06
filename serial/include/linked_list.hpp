#if !defined(LINKED_LIST_HPP)
#define LINKED_LIST_HPP

template <typename T>
class LinkedList
{
public:
    struct Node
    {
        T *value;
        Node *next = nullptr;

        Node(T *value, Node *next = nullptr) : value(value), next(next) {}
    };

    Node *first = nullptr;
    Node *last = nullptr;

    ~LinkedList()
    {
        auto *cur = first;

        while (cur != nullptr)
        {
            auto temp = cur;
            cur = cur->next;
            delete temp;
        }
    }

    void append(T *value)
    {
        auto new_node = new Node(value);
        if (first == nullptr)
        {
            first = new_node;
            last = new_node;
            return;
        }

        last->next = new_node;
        last = new_node;
    }

    void remove(T *value)
    {
        // TODO: use only cur_ptr
        auto *cur = first;
        Node **cur_ptr = nullptr;

        while (cur && cur->value != value)
        {
            cur = cur->next;
            cur_ptr = &(cur->next);
        }

        if (cur != nullptr)
        {
            cur_ptr = cur->next;
        }
    }
};

#endif // LINKED_LIST_HPP
