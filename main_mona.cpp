// This file's encoding is UTF-8.

#include <baygui.h>

class W3MFrame: public Frame {
public:
  W3MFrame() {
     setBounds((800 - 212) / 2, (600 - 50) / 2, 212, 50);
     setTitle("w3m");
     m_label = new Label(m_time, Label::CENTER);
     m_label->setBounds(0, 4, 200, 16);
     add(m_label);
  }

  ~W3MFrame(){
      delete(m_label);
  }

private: 
    char m_time[128];
    Label *m_label;
};

int main(int argc, char* argv[]) {
    W3MFrame *frame = new W3MFrame();
    frame->run();
    delete(frame);
    return 0;
}

