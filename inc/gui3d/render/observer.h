/**
 * Create a Observer
 * This virtual function will be called upon receive of any event
 * after starting listening at any CObservable object.
 **/

#pragma once
#include <gui3d/utils/dependency.hpp>

namespace gui3d{
class Figure;

struct GuiObserver : public CObserver
{
    Figure* m_figure;      // listened
    bool  request_to_quit;
    bool showing_help, hiding_help;
    CTicTac  tim_show_start, tim_show_end;

    GuiObserver(Figure* fig) : m_figure(fig)
        , request_to_quit(false)
        , showing_help(false), hiding_help(false)
    {}

protected:
    virtual void OnEvent(const mrptEvent &e) MRPT_OVERRIDE; // Main Process
    void Help();
    void DoEvent(mrptEventWindowChar keycode);
};
typedef std::shared_ptr<GuiObserver> GuiObserverPtr;

} // namespace gui3d
