#ifndef TAGRFID_H

#define TAGRFID_H

#include <Arduino.h>

class TagRFID {
private:
    String uid;
    String bebida;
public:
    TagRFID(String uid, String bebida);
    void setUid(String uid);
    String getUid();
    void setBebida(String bebida);
    String getBebida();
};

#endif