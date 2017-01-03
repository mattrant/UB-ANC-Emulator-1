NS3ROOT = /home/jalil/workspace/ns-allinone-3.26/ns-3.26/build

LIBS += -lns3.26-core-debug \
    -lns3.26-network-debug \
    -lns3.26-internet-debug \
    -lns3.26-wifi-debug \
    -lns3.26-mobility-debug \
    -lns3.26-olsr-debug \
    -L$${NS3ROOT} \
    -Wl,-rpath,$${NS3ROOT} \

INCLUDEPATH += $${NS3ROOT} \

DEPENDPATH += $${NS3ROOT} \
