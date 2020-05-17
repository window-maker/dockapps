#ifndef WM_COLORS_H
#define WM_COLORS_H

#define SLICE(_int, _bit) (((_int) >> (_bit)) & 0xff)

class Color {
  unsigned char _R, _G, _B;

 public:
  Color(unsigned long int li = 0) 
    : _R(SLICE(li, 16)), _G(SLICE(li, 8)), _B(SLICE(li, 0)) { }
  Color(int R, int G, int B) : _R(R), _G(G), _B(B) { }
  
  bool operator ==   (const Color & c) const
    { return _R == c._R && _G == c._G && _B == c._B; }
  bool operator !=   (const Color & c) const
    { return ! operator==(c); }

  unsigned char r() const { return _R; }
  unsigned char g() const { return _G; }
  unsigned char b() const { return _B; }
  
  static Color alpha_blend(const Color & c1, const Color & c2, double weight1)
  {       
    double weight2 = 1.0 - weight1;
    int r = static_cast<int>(c1.r() * weight1 + c2.r() * weight2 + 0.5);
    int g = static_cast<int>(c1.g() * weight1 + c2.g() * weight2 + 0.5);
    int b = static_cast<int>(c1.b() * weight1 + c2.b() * weight2 + 0.5);
    return Color(r, g, b);
  }
};

#undef SLICE

#define WMColor(x) (WMApp::colormap[static_cast<int>(WMColor:: x)])

namespace WMColor {
  enum WMColor {
    Background, // Background of WMFrames and WMImages
    Dim,	// Dark drawing color
    Medium,	// Medium drawing color
    Bright,	// Light drawing color
    ButtonFace,	// Color of face of buttons
    ButtonBorderDim,	// Colors of borders of WMButtons
    ButtonBorderMedium,
    ButtonBorderBright,
    FrameBorderDim,	// Colors of borders of WMFrames
    FrameBorderMedium,
    FrameBorderBright
  };
  const int numcolors = 11;
}

#endif
