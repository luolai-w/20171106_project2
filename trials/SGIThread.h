#ifndef __SGTHREAD_H
#define __SGTHREAD_H

class SGIThread{
    public:
    SGIThread(){};
    virtual ~SGIThread(){};
    virtual int Initialize() = 0;
    virtual int Start() = 0;
    virtual int Join() = 0;
    virtual int Stop() = 0;

    protected:
    virtual int Run() = 0;
};

#endif
