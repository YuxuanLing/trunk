#ifndef TESTSEQUENCES_H
#define TESTSEQUENCES_H

typedef struct
{
  char path[256];
  int width;
  int height;
  int fps;
  int frames;
  double psnr_ref[4];
  double bitrate_ref[4];
} TestSequence;

static const TestSequence sequences_benchmark_v2 [] = {
  { .path = "BasketballPass_416x240_50.yuv",
  .width=416, .height=240, .fps=50, .frames=500,
  .psnr_ref={36.963, 34.217, 31.779, 29.554},
  .bitrate_ref={1499.302, 883.017, 501.760, 285.751}},

  { .path = "BQSquare_416x240_60.yuv",
  .width=416, .height=240, .fps=60, .frames=600,
  .psnr_ref={34.393, 31.285, 28.352, 25.767},
  .bitrate_ref={2456.631, 1163.662, 450.088, 195.990}},

  { .path = "RaceHorses_416x240_30.yuv",
  .width=416, .height=240, .fps=30, .frames=300,
  .psnr_ref={36.245, 33.248, 30.646, 28.543},
  .bitrate_ref={1108.862, 624.013, 332.882, 183.274}},

  { .path = "BlowingBubbles_416x240_50.yuv",
  .width=416, .height=240, .fps=50, .frames=500,
  .psnr_ref={34.118, 31.060, 28.361, 25.992},
  .bitrate_ref={1612.095, 787.644, 378.472, 183.525}},

  { .path = "BasketballDrill_832x480_50.yuv",
  .width=832, .height=480, .fps=50, .frames=500,
  .psnr_ref={36.289, 33.968, 31.846, 29.782},
  .bitrate_ref={3555.948, 1971.378, 1097.090, 636.198}},

  { .path = "BQMall_832x480_60.yuv",
  .width=832, .height=480, .fps=60, .frames=600,
  .psnr_ref={36.923, 34.347, 31.850, 29.452},
  .bitrate_ref={3566.295, 1980.443, 1119.890, 667.415}},

  { .path = "RaceHorses_832x480_30.yuv",
  .width=832, .height=480, .fps=30, .frames=300,
  .psnr_ref={36.835, 34.030, 31.447, 29.133},
  .bitrate_ref={4449.506, 2311.300, 1196.510, 641.743}},

  { .path = "PartyScene_832x480_50.yuv",
  .width=832, .height=480, .fps=50, .frames=500,
  .psnr_ref={34.398, 31.264, 28.441, 25.917},
  .bitrate_ref={8238.292, 4056.495, 1935.286, 940.838}},

  { .path = "Keiba_832x480_30.yuv",
  .width=832, .height=480, .fps=30, .frames=300,
  .psnr_ref={38.446, 36.174, 33.935, 31.764},
  .bitrate_ref={2192.934, 1279.859, 759.204, 478.401}},

  { .path = "BasketballDrive_1920x1080_50.yuv",
  .width=1920, .height=1080, .fps=50, .frames=500,
  .psnr_ref={37.280, 35.625, 33.858, 31.908},
  .bitrate_ref={15661.811, 8575.638, 5038.089, 3070.707}},

  { .path = "BQTerrace_1920x1080_60.yuv",
  .width=1920, .height=1080, .fps=60, .frames=600,
  .psnr_ref={35.267, 33.263, 31.132, 28.970},
  .bitrate_ref={20680.350, 7331.443, 3356.915, 1822.974}},

  { .path = "Cactus_1920x1080_50.yuv",
  .width=1920, .height=1080, .fps=50, .frames=500,
  .psnr_ref={36.492, 34.586, 32.628, 30.644},
  .bitrate_ref={12493.098, 6561.245, 3571.207, 2036.308}},

  { .path = "ParkScene_1920x1080_24.yuv",
  .width=1920, .height=1080, .fps=24, .frames=240,
  .psnr_ref={36.737, 34.260, 31.941, 29.841},
  .bitrate_ref={6007.868, 3095.269, 1626.318, 920.988}},

  { .path = "Kimono_1920x1080_24.yuv",
  .width=1920, .height=1080, .fps=24, .frames=240,
  .psnr_ref={39.103, 37.030, 34.939, 32.858},
  .bitrate_ref={5131.231, 2953.755, 1709.874, 1049.166}},

  { .path = "vidyo1_1280x720_60.yuv",
  .width=1280, .height=720, .fps=60, .frames=600,
  .psnr_ref={40.119, 37.889, 35.647, 33.436},
  .bitrate_ref={1377.895, 756.681, 457.883, 300.833}},

  { .path = "vidyo3_1280x720_60.yuv",
  .width=1280, .height=720, .fps=60, .frames=600,
  .psnr_ref={40.061, 37.578, 35.003, 32.530},
  .bitrate_ref={1745.342, 878.304, 493.830, 291.434}},

  { .path = "vidyo4_1280x720_60.yuv",
  .width=1280, .height=720, .fps=60, .frames=600,
  .psnr_ref={39.909, 37.670, 35.509, 33.312},
  .bitrate_ref={1463.606, 753.903, 443.529, 291.171}},

  { .path = "ChangeSeats_1920x1080_60.yuv",
  .width=1920, .height=1080, .fps=60, .frames=600,
  .psnr_ref={39.967, 38.253, 36.363, 34.203},
  .bitrate_ref={8425.141, 3952.315, 2256.162, 1454.856}},

  { .path = "HeadAndShoulder_1920x1080_60.yuv",
  .width=1920, .height=1080, .fps=60, .frames=600,
  .psnr_ref={39.291, 37.991, 36.669, 35.016},
  .bitrate_ref={2599.647, 1027.650, 573.779, 388.585}},

  { .path = "SittingDown_1920x1080_60.yuv",
  .width=1920, .height=1080, .fps=60, .frames=600,
  .psnr_ref={40.448, 38.774, 36.920, 34.796},
  .bitrate_ref={6358.264, 3054.686, 1775.916, 1157.095}},

  { .path = "TelePresence_1920x1080_60.yuv",
  .width=1920, .height=1080, .fps=60, .frames=600,
  .psnr_ref={41.405, 39.981, 38.234, 35.731},
  .bitrate_ref={3756.394, 1902.157, 1182.159, 799.890}},

  { .path = "WhiteBoard_1920x1080_60.yuv",
  .width=1920, .height=1080, .fps=60, .frames=600,
  .psnr_ref={39.897, 38.125, 36.143, 33.912},
  .bitrate_ref={4816.219, 1880.067, 1047.128, 687.738}},
};


static const TestSequence sequences_playtime [] = {
  { .path = "../raw/presentation/presentation-pdf_1280x720.yuv",
  .width=1280, .height=720, .fps=15, .frames=150,
  .psnr_ref={41.681, 38.015, 34.576, 31.097},
  .bitrate_ref={191.394, 151.064, 110.525, 81.680}},

  { .path = "../raw/presentation/terminal-scroll_1280x720.yuv",
  .width=1280, .height=720, .fps=15, .frames=150,
  .psnr_ref={35.346, 32.066, 28.363, 24.849},
  .bitrate_ref={2959.697, 2200.911, 1572.450, 1068.182}},

  { .path = "../raw/presentation/terminal-static_1280x720.yuv",
  .width=1280, .height=720, .fps=15, .frames=150,
  .psnr_ref={37.325, 33.999, 30.496, 27.133},
  .bitrate_ref={193.750, 145.179, 109.546, 73.698}},

  { .path = "BasketballPass_416x240_50.yuv",
  .width=416, .height=240, .fps=50, .frames=500,
  .psnr_ref={36.161, 33.355, 30.876, 28.487},
  .bitrate_ref={1550.734, 912.206, 514.968, 299.799}},

  { .path = "BQSquare_416x240_60.yuv",
  .width=416, .height=240, .fps=60, .frames=600,
  .psnr_ref={33.705, 30.556, 27.596, 24.900},
  .bitrate_ref={3157.234, 1604.574, 670.952, 266.710}},

  { .path = "RaceHorses_416x240_30.yuv",
  .width=416, .height=240, .fps=30, .frames=300,
  .psnr_ref={35.396, 32.425, 29.889, 27.682},
  .bitrate_ref={1161.558, 643.566, 351.390, 200.804}},

  { .path = "BlowingBubbles_416x240_50.yuv",
  .width=416, .height=240, .fps=50, .frames=500,
  .psnr_ref={33.546, 30.449, 27.809, 25.458},
  .bitrate_ref={1779.493, 857.946, 403.062, 193.202}},

  { .path = "BasketballDrill_832x480_50.yuv",
  .width=832, .height=480, .fps=50, .frames=500,
  .psnr_ref={35.804, 33.336, 31.082, 28.811},
  .bitrate_ref={3769.308, 2060.033, 1137.577, 674.203}},

  { .path = "BQMall_832x480_60.yuv",
  .width=832, .height=480, .fps=60, .frames=600,
  .psnr_ref={36.255, 33.490, 30.901, 28.270},
  .bitrate_ref={3883.882, 2129.051, 1210.931, 741.895}},

  { .path = "RaceHorses_832x480_30.yuv",
  .width=832, .height=480, .fps=30, .frames=300,
  .psnr_ref={36.013, 33.146, 30.555, 28.099},
  .bitrate_ref={4531.514, 2389.083, 1273.119, 747.925}},

  { .path = "PartyScene_832x480_50.yuv",
  .width=832, .height=480, .fps=50, .frames=500,
  .psnr_ref={33.643, 30.468, 27.705, 25.212},
  .bitrate_ref={9203.751, 4620.210, 2190.254, 1046.590}},

  { .path = "Keiba_832x480_30.yuv",
  .width=832, .height=480, .fps=30, .frames=300,
  .psnr_ref={37.676, 35.174, 32.722, 29.944},
  .bitrate_ref={2506.840, 1534.523, 987.814, 662.896}},

  { .path = "BasketballDrive_1920x1080_50.yuv",
  .width=1920, .height=1080, .fps=50, .frames=500,
  .psnr_ref={36.787, 34.783, 32.604, 30.045},
  .bitrate_ref={16641.477, 9198.685, 5413.213, 3338.941}},

  { .path = "BQTerrace_1920x1080_60.yuv",
  .width=1920, .height=1080, .fps=60, .frames=600,
  .psnr_ref={34.903, 32.690, 30.345, 27.821},
  .bitrate_ref={21909.603, 8504.557, 4016.277, 2173.014}},

  { .path = "Cactus_1920x1080_50.yuv",
  .width=1920, .height=1080, .fps=50, .frames=500,
  .psnr_ref={36.220, 34.108, 31.976, 29.713},
  .bitrate_ref={13096.624, 6812.184, 3713.309, 2124.882}},

  { .path = "ParkScene_1920x1080_24.yuv",
  .width=1920, .height=1080, .fps=24, .frames=240,
  .psnr_ref={36.253, 33.644, 31.269, 28.870},
  .bitrate_ref={6115.530, 3129.470, 1684.873, 1004.222}},

  { .path = "Kimono_1920x1080_24.yuv",
  .width=1920, .height=1080, .fps=24, .frames=240,
  .psnr_ref={38.440, 36.138, 33.817, 31.170},
  .bitrate_ref={5010.955, 2895.574, 1740.370, 1087.538}},

  { .path = "vidyo1_1280x720_60.yuv",
  .width=1280, .height=720, .fps=60, .frames=600,
  .psnr_ref={39.398, 36.848, 34.526, 32.075},
  .bitrate_ref={1492.677, 787.494, 468.163, 295.190}},

  { .path = "vidyo3_1280x720_60.yuv",
  .width=1280, .height=720, .fps=60, .frames=600,
  .psnr_ref={39.303, 36.550, 33.967, 31.451},
  .bitrate_ref={1994.238, 932.710, 521.049, 325.315}},

  { .path = "vidyo4_1280x720_60.yuv",
  .width=1280, .height=720, .fps=60, .frames=600,
  .psnr_ref={39.259, 36.814, 34.478, 31.906},
  .bitrate_ref={1610.181, 798.758, 456.735, 293.186}},

  { .path = "ChangeSeats_1920x1080_60.yuv",
  .width=1920, .height=1080, .fps=60, .frames=600,
  .psnr_ref={39.323, 37.049, 34.546, 31.684},
  .bitrate_ref={8299.708, 4070.954, 2311.975, 1507.241}},

  { .path = "HeadAndShoulder_1920x1080_60.yuv",
  .width=1920, .height=1080, .fps=60, .frames=600,
  .psnr_ref={39.097, 37.561, 35.830, 33.562},
  .bitrate_ref={3233.067, 1300.914, 645.011, 418.582}},

  { .path = "SittingDown_1920x1080_60.yuv",
  .width=1920, .height=1080, .fps=60, .frames=600,
  .psnr_ref={39.807, 37.567, 35.008, 32.058},
  .bitrate_ref={6375.286, 3182.843, 1826.118, 1206.033}},

  { .path = "TelePresence_1920x1080_60.yuv",
  .width=1920, .height=1080, .fps=60, .frames=600,
  .psnr_ref={40.728, 37.822, 35.267, 32.337},
  .bitrate_ref={3807.497, 2025.173, 1262.834, 889.650}},

  { .path = "WhiteBoard_1920x1080_60.yuv",
  .width=1920, .height=1080, .fps=60, .frames=600,
  .psnr_ref={39.410, 37.123, 34.633, 31.912},
  .bitrate_ref={5005.108, 2067.162, 1089.652, 715.242}},
};


#endif
