#include <iostream>
#include <unordered_map>

class Cache_LFU {
    struct frqListNode;//提前声明，因为frqListNode是在keyListNode后面声明的

    struct keyListNode{//双向链表，并且除了key和value以外还会保存频率节点的指针
        frqListNode* frq;

        keyListNode* prev;
        keyListNode* next;
        
        int key;
        int value;

        void getOut() {//将节点从链表中移除
            prev->next = next;
            next->prev = prev;

            frq->size -= 1;
        }

        keyListNode(int key_ = 0, int value_ = 0) : key(key_), value(value_) {
            prev = next = nullptr;
            frq = nullptr;
        }
    };

    struct frqListNode{//单向链表，保存了相同频率的key节点
        frqListNode* next;

        keyListNode* head;
        keyListNode* tail;

        int size;

        frqListNode() : size(0) {
            head = new keyListNode();
            tail = new keyListNode();
            head->next = tail;
            tail->prev = head;
        }

        frqListNode* getNext() {//获取下一个频率节点，如果没有则创建一个
            if(!next) next = new frqListNode();

            return next;
        }

        void insert(keyListNode* node) {//将key节点插入到链表中
            node->next = head->next;
            head->next->prev = node;
            head->next = node;
            node->prev = head;

            node->frq = this;
            size += 1;
        }

        ~frqListNode() {
            keyListNode* cur = head;
            while(cur) {
                keyListNode* next = cur->next;
                delete cur;
                cur = next;
            }
        }
    };

    frqListNode* head;//频率节点链表的头节点
    frqListNode* minfrq;//保存最小频率的节点
    std::unordered_map<int, keyListNode*> keyNodeMap;//保存key节点的map

    int capacity;
    int size;
    int ageing_time;//老化时间
    int cur_time;//当前时间

    void update(int key) {//更新key节点的频率
        keyListNode* node = keyNodeMap[key];
        node->getOut();//将节点从原来的频率节点中移除

        frqListNode* frq = node->frq;//获取当前频率节点
        frqListNode* nextfrq = frq->getNext();//获取下一个频率节点
        nextfrq->insert(node);//将节点插入到下一个频率节点中

        if(frq->size == 0 && frq == minfrq) {//如果更新后原本频率节点为空，并且是最小频率节点，则更新最小频率节点
            minfrq = nextfrq;
        }
    }

    void pop() {//删除最小频率节点的尾节点，此处不需要判断是否更新minfrq，因为超出容量的情况下，minfrq一定会更新为head
        keyListNode* node = minfrq->tail->prev;
        keyNodeMap.erase(node->key);
        node->getOut();
        delete node;
        size -= 1;
    }

    void insert(int key, int value) {//插入新的key节点
        if(capacity == 0) return;

        if(size == capacity) pop();//如果超出容量，则删除最小频率节点的尾节点

        keyListNode* node = new keyListNode(key, value);
        keyNodeMap[key] = node;
        head->insert(node);

        minfrq = head;//插入新节点后，最小频率一定会更新为1

        size += 1;
    }

    void ageing() {//老化操作
        if(cur_time < ageing_time) {//如果当前时间小于老化时间，则直接加1
            cur_time += 1;
            return;    
        }

        cur_time = 0;
        frqListNode* nextfrq = head->next;//获取下一个频率节点

        if(minfrq == head) {//如果最小频率节点是head，则直接更新minfrq为下一个频率节点
            minfrq = nextfrq;

            //将head的节点插入到下一个频率节点的尾部
            head->tail->prev->next = nextfrq->tail;
            head->head->next->prev = nextfrq->tail->prev;

            nextfrq->tail->prev->next = head->head->next;
            nextfrq->tail->prev = head->tail->prev;
        }

        delete head;
        head = nextfrq;
    }

    public:
    Cache_LFU(int capacity_, int ageing_time_) : capacity(capacity_), size(0), ageing_time(ageing_time_), cur_time(0){
        head = new frqListNode();
        minfrq= head;
    }

    int get(int key) {//获取key节点的value
        ageing();
        if(keyNodeMap.find(key) == keyNodeMap.end()) return -1;

        update(key);
        return keyNodeMap[key]->value;
    }

    void put(int key, int value) {//插入或更新key节点
        ageing();
        if(capacity == 0) return;

        if(keyNodeMap.find(key) != keyNodeMap.end()) {//如果key节点已经存在，则更新value
            keyNodeMap[key]->value = value;
            update(key);
            return;
        }

        insert(key, value);//插入新的key节点
    }
};