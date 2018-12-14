#pragma once

#include <gui3d/base/type_def.h>

namespace matlab{

// Plot same as matlab
/* The lineFormat string is a combination of the following characters:
* - Line styles:
*		- '.': One point for each data point
*		- '-': A continuous line
*		- ':': A dashed line
* - Colors:
*		- k: black
*		- r: red
*		- g: green
*		- b: blue
*		- m: magenta
*		- c: cyan
* - Line width:
*		- '1' to '9': The line width (default=1)
*
*  Examples:
*   - 'r.' -> red points.
*   - 'k3' or 'k-3' -> A black line with a line width of 3 pixels.
*/
using gui3d::hObject;
using std::vector;
using std::string;

const hObject figure(const string& plot_name = string());
const hObject internal_plot(const vector<float>& x, const vector<float >& y,
                            const string& format, const string& plot_name);
void          internal_plot(hObject fig, const vector<float>& x, const vector<float >& y, const string& format);

template <typename VECTOR1,typename VECTOR2>
hObject plot(const VECTOR1 &x,const VECTOR2 &y,const std::string &lineFormat,const std::string &plotName = string())
{
    vector<float> x1(x.size()), y1(y.size());
    const size_t N1=size_t(x.size());
    for (size_t i=0;i<N1;i++) x1[i]=x[i];
    const size_t N2=size_t(y.size());
    for (size_t i=0;i<N2;i++) y1[i]=y[i];
    return internal_plot(x1,y1,lineFormat,plotName);
}

template <typename VECTOR1>
hObject plot(const VECTOR1 &y,const std::string  &lineFormat,const std::string  &plotName = string())
{
    const size_t N=size_t(y.size());
    vector<float> x1(N),y1(N);
    for (size_t i=0;i<N;i++) { x1[i]=i; y1[i]=y[i]; }
    return internal_plot(x1,y1,lineFormat,plotName);
}

template <typename VECTOR1,typename VECTOR2>
void plot(hObject fig, const VECTOR1 &x,const VECTOR2 &y,const std::string &lineFormat)
{
    vector<float> x1(x.size()), y1(y.size());
    const size_t N1=size_t(x.size());
    for (size_t i=0;i<N1;i++) x1[i]=x[i];
    const size_t N2=size_t(y.size());
    for (size_t i=0;i<N2;i++) y1[i]=y[i];
    internal_plot(fig,x1,y1,lineFormat);
}

template <typename VECTOR1>
void plot(hObject fig, const VECTOR1 &y,const std::string &lineFormat,const std::string &plotName)
{
    const size_t N=size_t(y.size());
    vector<float> x1(N),y1(N);
    for (size_t i=0;i<N;i++) { x1[i]=i; y1[i]=y[i]; }
    internal_plot(fig,x1,y1,lineFormat);
}

void axis_limit(hObject fig, float x_min, float x_max, float y_min, float y_max);
void axis_equal(hObject fig);
void axis_fit  (hObject fig);
void hold_on   (hObject fig);
void hold_off  (hObject fig);

}
