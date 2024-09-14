// Xbox
DEFINE_ENUM(ButtonA, 0, A)
DEFINE_ENUM(ButtonB, 1, B)
DEFINE_ENUM(ButtonX, 2, X)
DEFINE_ENUM(ButtonY, 3, Y)

// Playstation
DEFINE_ENUM_ALT(ButtonCross, 0, Cross)
DEFINE_ENUM_ALT(ButtonCircle, 1, Circle)
DEFINE_ENUM_ALT(ButtonSquare, 2, Square)
DEFINE_ENUM_ALT(ButtonTriangle, 3, Triangle)

// Xbox
DEFINE_ENUM(ButtonLB, 4, LB)
DEFINE_ENUM(ButtonLT, 5, LT)
DEFINE_ENUM(ButtonRB, 9, RB)
DEFINE_ENUM(ButtonRT, 10, RT)
DEFINE_ENUM(ButtonLSB, 6, LSB)
DEFINE_ENUM(ButtonRSB, 11, RSB)

// Playstation
DEFINE_ENUM_ALT(ButtonL1, 4, L1)
DEFINE_ENUM_ALT(ButtonL2, 5, L2)
DEFINE_ENUM_ALT(ButtonL3, 6, L3)

DEFINE_ENUM_ALT(ButtonR1, 9, R1)
DEFINE_ENUM_ALT(ButtonR2, 10, R2)
DEFINE_ENUM_ALT(ButtonR3, 11, R3)

// Xbox
DEFINE_ENUM(ButtonBack, 14, Back)
DEFINE_ENUM(ButtonStart, 15, Start)

// Playstation
DEFINE_ENUM_ALT(ButtonShare, 14, Share)
DEFINE_ENUM_ALT(ButtonOptions, 15, Options)
DEFINE_ENUM_ALT(ButtonSelect, 14, Select)
DEFINE_ENUM(ButtonHome, 16, Home)

// DPad
DEFINE_ENUM(ButtonDPadUp, 17,			DPadUp)
DEFINE_ENUM(ButtonDPadRight, 18,		DPadRight)
DEFINE_ENUM(ButtonDPadDown, 19,			DPadDown)
DEFINE_ENUM(ButtonDPadLeft, 20,			DPadLeft)

// Left Analog
DEFINE_ENUM(ButtonLeftAnalogUp, 21,		LeftAnalogUp)
DEFINE_ENUM(ButtonLeftAnalogRight, 22,	LeftAnalogRight)
DEFINE_ENUM(ButtonLeftAnalogDown, 23,	LeftAnalogDown)
DEFINE_ENUM(ButtonLeftAnalogLeft, 24,	LeftAnalogLeft)

// Right Analog
DEFINE_ENUM(ButtonRightAnalogUp, 25,	RightAnalogUp)
DEFINE_ENUM(ButtonRightAnalogRight, 26,	RightAnalogRight)
DEFINE_ENUM(ButtonRightAnalogDown, 27,	RightAnalogDown)
DEFINE_ENUM(ButtonRightAnalogLeft, 28,	RightAnalogLeft)

DEFINE_ENUM_INFO(ButtonCount, ButtonDPadUp) // TODO: Remove unused space (7+8)
DEFINE_ENUM_INFO(InputCount, 29) // TODO: Remove unused space (7+8)

DEFINE_ENUM_INFO(Max, 32)
