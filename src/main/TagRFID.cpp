#include "TagRFID.h"

TagRFID::TagRFID(String uid, String bebida) {
  this->uid = uid;
  this->bebida = bebida;
}

void TagRFID::setUid(String uid) {
  this->uid = uid;
}

String TagRFID::getUid() {
  return uid;
}

void TagRFID::setBebida(String bebida) {
  this->bebida = bebida;
}

String TagRFID::getBebida() {
  return bebida;
}
