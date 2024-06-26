# -lfu-
用一个单向链表、一个双向链表和一个哈希表实现的随操作次数，频率自动老化的LFU缓存<br>
用单向链表表示频率，并挂载一个保存所有此频率的key节点的双向链表<br>
维护一个minfrq指针指向最小频率节点，head指针指向频率链表头节点<br>
每次操作都会使被操作的key节点进入下一个频率节点<br>
如果进入下一个频率的key节点是原本频率唯一的key节点，并且原本频率是minfrq，就更新minfrq<br>
缓存遗弃最最久未使用的key节点时，从minfrq中移除尾节点并将minfrq更新为head，因为新插入的节点频率必定是1，会加入到head频率中<br>

# 老化操作
维护一个cur_time，每次操作时cur_time加一<br>
当cur_time增加到ageing_time时，cur_time归零并触发一次老化操作<br>
如果head频率中有key节点，那么head频率一定是minfrq，此时将head频率的所有key节点移入下一频率，并更新head和minfrq到下一频率，释放原本频率节点<br>
如果head中没有key节点，那么直接释放原本频率节点并更新head<br>
相当于所有数据的频率都减少一次<br>
避免早期的热点老数据长期没有再访问的情况下依然占据cache空间<br>
例如极端情况下，最近两个新的热点数据被交替访问，却因为初始频率为1，从而交替将对方挤出缓存，使得缓存机制完全失效，形同虚设<br>
