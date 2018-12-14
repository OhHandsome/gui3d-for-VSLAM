#include <gui3d/gui.h>

#define CurCameraOptions        tOptions(OrangeRed, 1, tOptions::HOLD_OFF)
#define PrediectCameraOptions   tOptions(OrangeRed, 0.5, tOptions::HOLD_OFF)
#define GTCameraOptions         tOptions(Red, 0.5, tOptions::HOLD_OFF)
#define RefMapPointsOptions     tOptions(Red, 1, tOptions::HOLD_OFF)
#define GlobalMapPointsOptions  tOptions(Yellow, 1, tOptions::HOLD_OFF)
#define LocalFramesOptions      tOptions(Green, 0.25, tOptions::HOLD_OFF)
#define KeyFramesOptions        tOptions(Cyan,  0.25, tOptions::HOLD_OFF)
#define DepthPointCloudOptions  tOptions(White,  1.1f, tOptions::HOLD_OFF)
#define Model3dOptions          tOptions(White,  1.0f, tOptions::HOLD_OFF)
#define PathOptions             tOptions(Orange, 1.0f, tOptions::HOLD_OFF)

namespace gui3d{

std::multimap<Channel, tOptions> s_option_table;
void registerSystemChannelOptions()
{
    static bool bregister = false;
    if(bregister)  return;

    s_option_table.clear();
    #define RegisterOptions(CH)  s_option_table.insert(std::make_pair(sysChannel[CH], CH##Options))
    RegisterOptions(CurCamera);
    RegisterOptions(PrediectCamera);
    RegisterOptions(GTCamera);
    RegisterOptions(RefMapPoints);
    RegisterOptions(GlobalMapPoints);
    RegisterOptions(LocalFrames);
    RegisterOptions(KeyFrames);
    RegisterOptions(Path);
    RegisterOptions(DepthPointCloud);
    RegisterOptions(Model3d);
    assert(s_option_table.size() == Model3d + 1);
    bregister = true;
}

bool systemChannelOptions(const Channel& name, tOptions& options)
{
    auto it = s_option_table.find(name);
    if(it == s_option_table.end())
        return false;

    options = it->second;
    return true;
}

}// namespace gui3d

