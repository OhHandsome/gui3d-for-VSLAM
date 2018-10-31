#include <gui3d/utils/dependency.hpp>
#include <gui3d/plot.h>
#include <gui3d/base/log.h>

namespace matlab{

typedef CDisplayWindowPlots Figure2d;
typedef CDisplayWindowPlotsPtr Figure2dPtr;
std::vector<CDisplayWindowPlotsPtr> sSystemFigure2d;
char color[] = "krgbmc";

void MenuCallback(int menuID, float x, float y, void *userParam)
{
    Figure2d* win = (Figure2d *)(userParam);
    switch (menuID)
    {
        case 1:
            LOGI("[X Y]: %f,%f", x, y);
            break;

        case 2:
            win->clear();
            break;

        default:
            break;
    }
}

const hObject figure(const string& plot_name)
{
    std::string name;
    if(plot_name.empty())
        name = format("%ld", sSystemFigure2d.size() + 1);
    else
        name = plot_name;

    Figure2dPtr win = CDisplayWindowPlotsPtr(new CDisplayWindowPlots(name, 640, 480));
    win->enableMousePanZoom(true);
    win->addPopupMenuEntry("Mark this point...", 1);
    win->addPopupMenuEntry("clear lines", 2);
    win->setMenuCallback(MenuCallback, (void *) win.get());

    sSystemFigure2d.push_back(win);
    return (hObject)(win.get());
}

const hObject internal_plot(const vector<float>& x, const vector<float >& y, const string& format, const string& plot_name)
{
    hObject fig = figure(plot_name);
    Figure2d* win = (Figure2d*)fig;

    win->plot(x, y, format);
    auto y_max = std::max_element(std::begin(y), std::end(y));
    auto y_min = std::min_element(std::begin(y), std::end(y));
    auto x_max = std::max_element(std::begin(x), std::end(x));
    auto x_min = std::min_element(std::begin(x), std::end(x));

    float x0 = (*x_max + *x_min) * 0.5f;
    float y0 = (*y_max + *y_min) * 0.5f;
    float width = (*x_max - *x_min) * 0.5f;
    float height = (*y_max - *y_min) * 0.5f;

    win->axis(x0 - 1.2f * width, x0 + 1.2f * width,
              y0 - 1.2f * height, y0 + 1.2f * height);

    win->hold_off();
    return fig;
}

void internal_plot(hObject fig, const vector<float>& x, const vector<float >& y, const string& format)
{
    if(!fig) return;
    Figure2d* win = (Figure2d*)fig;
    win->plot(x, y, format);
    win->hold_off();
}


void axis_limit(const hObject fig, float x_min, float x_max, float y_min, float y_max)
{
    if(!fig) return;
    Figure2d* win = (Figure2d*)fig;
    win->axis(x_min, x_max, y_min, y_max);
}

void axis_equal(const hObject fig)
{
    if(!fig) return;
    Figure2d* win = (Figure2d*)fig;
    win->axis_equal(true);
}

void axis_fit(const hObject fig)
{
    if(!fig) return;
    Figure2d* win = (Figure2d*)fig;
    win->axis_fit();
}

void hold_on(const hObject fig)
{
    if(!fig) return;
    Figure2d* win = (Figure2d*)fig;
    win->hold_on();
}

void hold_off(const hObject fig)
{
    if(!fig) return;
    Figure2d* win = (Figure2d*)fig;
    win->hold_off();
}

}// namespace matlab
