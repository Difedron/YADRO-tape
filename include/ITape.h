//
// Created by Diana Kholukhina on 10.05.2026.
//

#ifndef YADRO_TAPE_ITAPE_H
#define YADRO_TAPE_ITAPE_H

class ITape {
public:
    virtual int read() = 0;
    virtual void write(int value) = 0;

    virtual void moveLeft() = 0;
    virtual void moveRight() = 0;
    virtual void rewind() = 0;

    virtual bool isEnd() const = 0;

    virtual ~ITape() = default;
};

#endif // YADRO_TAPE_ITAPE_H
