xof 0303txt 0032
template FVFData {
 <b6e70a0e-8ef9-4e83-94ad-ecc8b0c04897>
 DWORD dwFVF;
 DWORD nDWords;
 array DWORD data[nDWords];
}

template EffectInstance {
 <e331f7e4-0559-4cc2-8e99-1cec1657928f>
 STRING EffectFilename;
 [...]
}

template EffectParamFloats {
 <3014b9a0-62f5-478c-9b86-e4ac9f4e418b>
 STRING ParamName;
 DWORD nFloats;
 array FLOAT Floats[nFloats];
}

template EffectParamString {
 <1dbc4c88-94c1-46ee-9076-2c28818c9481>
 STRING ParamName;
 STRING Value;
}

template EffectParamDWord {
 <e13963bc-ae51-4c5d-b00f-cfa3a9d97ce5>
 STRING ParamName;
 DWORD Value;
}


Material PDX03_-_Default {
 0.137255;0.156863;0.180392;1.000000;;
 9.599999;
 0.288627;0.304314;0.329412;;
 0.000000;0.000000;0.000000;;
}

Material PDX01_-_Default {
 0.643137;0.643137;0.643137;1.000000;;
 3.200000;
 0.000000;0.000000;0.000000;;
 0.000000;0.000000;0.000000;;
}

Frame Object00 {
 

 FrameTransformMatrix {
  1.000000,-0.000000,0.000000,0.000000,-0.000000,-1.000000,-0.000000,0.000000,-0.000000,-0.000000,1.000000,0.000000,0.000000,-0.001000,-0.200000,1.000000;;
 }

 Frame {
  

  FrameTransformMatrix {
   1.000000,0.000000,-0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000;;
  }

  Mesh  {
   86;
   -0.920000;0.999000;0.304000;,
   -0.920000;0.999000;0.096000;,
   0.920000;0.999000;0.304000;,
   0.920000;0.999000;0.096000;,
   -0.980000;-0.981000;0.400000;,
   0.980000;-0.981000;0.400000;,
   -0.980000;0.979000;0.400000;,
   0.980000;0.979000;0.400000;,
   -0.980000;-0.995142;0.394142;,
   -0.990000;-0.991000;0.394142;,
   -0.994142;-0.981000;0.394142;,
   -0.994142;0.979000;0.394142;,
   -0.990000;0.989000;0.394142;,
   -0.980000;0.993142;0.394142;,
   0.980000;0.993142;0.394142;,
   0.990000;0.989000;0.394142;,
   0.994142;0.979000;0.394142;,
   0.994142;-0.981000;0.394142;,
   0.990000;-0.991000;0.394142;,
   0.980000;-0.995142;0.394142;,
   -0.980000;-1.001000;0.380000;,
   -0.994142;-0.995142;0.380000;,
   -1.000000;-0.981000;0.380000;,
   -1.000000;0.979000;0.380000;,
   -0.994142;0.993142;0.380000;,
   -0.980000;0.999000;0.380000;,
   0.980000;0.999000;0.380000;,
   0.994142;0.993142;0.380000;,
   1.000000;0.979000;0.380000;,
   1.000000;-0.981000;0.380000;,
   0.994142;-0.995142;0.380000;,
   0.980000;-1.001000;0.380000;,
   -0.980000;-1.001000;0.020000;,
   -0.994142;-0.995142;0.020000;,
   -1.000000;-0.981000;0.020000;,
   -1.000000;0.979000;0.020000;,
   -0.994142;0.993142;0.020000;,
   -0.980000;0.999000;0.020000;,
   0.980000;0.999000;0.020000;,
   0.994142;0.993142;0.020000;,
   1.000000;0.979000;0.020000;,
   1.000000;-0.981000;0.020000;,
   0.994142;-0.995142;0.020000;,
   0.980000;-1.001000;0.020000;,
   -0.980000;-0.995142;0.005858;,
   -0.990000;-0.991000;0.005858;,
   -0.994142;-0.981000;0.005858;,
   -0.994142;0.979000;0.005858;,
   -0.990000;0.989000;0.005858;,
   -0.980000;0.993142;0.005858;,
   0.980000;0.993142;0.005858;,
   0.990000;0.989000;0.005858;,
   0.994142;0.979000;0.005858;,
   0.994142;-0.981000;0.005858;,
   0.990000;-0.991000;0.005858;,
   0.980000;-0.995142;0.005858;,
   0.980000;0.979000;0.000000;,
   -0.980000;0.979000;0.000000;,
   0.980000;-0.981000;-0.000000;,
   -0.980000;-0.981000;-0.000000;,
   -0.980000;-1.001000;0.380000;,
   -0.994142;-0.995142;0.380000;,
   -1.000000;-0.981000;0.380000;,
   -1.000000;0.979000;0.380000;,
   -0.994142;0.993142;0.380000;,
   -0.980000;0.999000;0.380000;,
   0.980000;0.999000;0.380000;,
   0.994142;0.993142;0.380000;,
   1.000000;0.979000;0.380000;,
   1.000000;-0.981000;0.380000;,
   0.994142;-0.995142;0.380000;,
   0.980000;-1.001000;0.380000;,
   -0.980000;-1.001000;0.020000;,
   -0.980000;-1.001000;0.380000;,
   -0.980000;-1.001000;0.020000;,
   -0.994142;-0.995142;0.020000;,
   -1.000000;-0.981000;0.020000;,
   -1.000000;0.979000;0.020000;,
   -0.994142;0.993142;0.020000;,
   -0.980000;0.999000;0.020000;,
   0.980000;0.999000;0.020000;,
   0.994142;0.993142;0.020000;,
   1.000000;0.979000;0.020000;,
   1.000000;-0.981000;0.020000;,
   0.994142;-0.995142;0.020000;,
   0.980000;-1.001000;0.020000;;
   116;
   3;4,6,7;,
   3;4,7,5;,
   3;4,8,9;,
   3;4,9,10;,
   3;4,10,11;,
   3;4,11,6;,
   3;6,11,12;,
   3;6,12,13;,
   3;6,13,14;,
   3;6,14,7;,
   3;7,14,15;,
   3;7,15,16;,
   3;7,16,17;,
   3;7,17,5;,
   3;5,17,18;,
   3;5,18,19;,
   3;5,19,8;,
   3;5,8,4;,
   3;8,20,21;,
   3;8,21,9;,
   3;9,21,22;,
   3;9,22,10;,
   3;10,22,23;,
   3;10,23,11;,
   3;11,23,24;,
   3;11,24,12;,
   3;12,24,25;,
   3;12,25,13;,
   3;13,25,26;,
   3;13,26,14;,
   3;14,26,27;,
   3;14,27,15;,
   3;15,27,28;,
   3;15,28,16;,
   3;16,28,29;,
   3;16,29,17;,
   3;17,29,30;,
   3;17,30,18;,
   3;18,30,31;,
   3;18,31,19;,
   3;19,31,20;,
   3;19,20,8;,
   3;60,32,33;,
   3;60,33,61;,
   3;61,33,34;,
   3;61,34,62;,
   3;62,34,35;,
   3;62,35,63;,
   3;63,35,36;,
   3;63,36,64;,
   3;64,36,37;,
   3;64,37,65;,
   3;66,38,39;,
   3;66,39,67;,
   3;67,39,40;,
   3;67,40,68;,
   3;68,40,41;,
   3;68,41,69;,
   3;69,41,42;,
   3;69,42,70;,
   3;70,42,43;,
   3;70,43,71;,
   3;71,43,72;,
   3;71,72,73;,
   3;74,44,45;,
   3;74,45,75;,
   3;75,45,46;,
   3;75,46,76;,
   3;76,46,47;,
   3;76,47,77;,
   3;77,47,48;,
   3;77,48,78;,
   3;78,48,49;,
   3;78,49,79;,
   3;79,49,50;,
   3;79,50,80;,
   3;80,50,51;,
   3;80,51,81;,
   3;81,51,52;,
   3;81,52,82;,
   3;82,52,53;,
   3;82,53,83;,
   3;83,53,54;,
   3;83,54,84;,
   3;84,54,55;,
   3;84,55,85;,
   3;85,55,44;,
   3;85,44,74;,
   3;44,59,45;,
   3;45,59,46;,
   3;46,59,57;,
   3;46,57,47;,
   3;47,57,48;,
   3;48,57,49;,
   3;49,57,56;,
   3;49,56,50;,
   3;50,56,51;,
   3;51,56,52;,
   3;52,56,58;,
   3;52,58,53;,
   3;53,58,54;,
   3;54,58,55;,
   3;55,58,59;,
   3;55,59,44;,
   3;58,56,57;,
   3;58,57,59;,
   3;25,37,0;,
   3;37,1,0;,
   3;1,37,38;,
   3;1,38,3;,
   3;3,38,26;,
   3;3,26,2;,
   3;2,26,25;,
   3;2,25,0;,
   3;2,1,3;,
   3;2,0,1;;

   MeshNormals  {
    65;
    -0.165727;-0.165727;0.972147;,
    -0.165728;0.165728;0.972146;,
    0.165727;0.165727;0.972147;,
    0.165728;-0.165728;0.972146;,
    -0.144765;-0.719357;0.679389;,
    -0.529353;-0.529349;0.663005;,
    -0.719358;-0.144764;0.679388;,
    -0.719357;0.144767;0.679388;,
    -0.529349;0.529353;0.663005;,
    -0.144766;0.719358;0.679388;,
    0.144765;0.719357;0.679389;,
    0.529353;0.529349;0.663005;,
    0.719358;0.144764;0.679388;,
    0.719357;-0.144767;0.679388;,
    0.529349;-0.529354;0.663004;,
    0.144766;-0.719359;0.679386;,
    -0.187691;-0.964129;0.187691;,
    -0.694694;-0.694691;0.186564;,
    -0.964129;-0.187689;0.187691;,
    -0.964128;0.187692;0.187691;,
    -0.694690;0.694694;0.186564;,
    -0.251175;0.934784;0.251175;,
    0.251175;0.934785;0.251175;,
    0.694694;0.694690;0.186564;,
    0.964129;0.187689;0.187691;,
    0.964128;-0.187693;0.187691;,
    0.694689;-0.694695;0.186564;,
    0.187690;-0.964129;0.187690;,
    -0.187690;-0.964129;-0.187691;,
    -0.694692;-0.694692;-0.186564;,
    -0.964129;-0.187689;-0.187691;,
    -0.964128;0.187693;-0.187692;,
    -0.694691;0.694693;-0.186564;,
    -0.251175;0.934785;-0.251175;,
    0.251175;0.934784;-0.251176;,
    0.694693;0.694691;-0.186564;,
    0.964129;0.187689;-0.187691;,
    0.964128;-0.187693;-0.187692;,
    0.694691;-0.694693;-0.186564;,
    0.187691;-0.964129;-0.187691;,
    -0.144766;-0.719359;-0.679386;,
    -0.529351;-0.529351;-0.663004;,
    -0.719357;-0.144763;-0.679389;,
    -0.719358;0.144768;-0.679387;,
    -0.529350;0.529352;-0.663005;,
    -0.144765;0.719357;-0.679388;,
    0.144766;0.719357;-0.679388;,
    0.529352;0.529350;-0.663005;,
    0.719357;0.144763;-0.679389;,
    0.719357;-0.144768;-0.679387;,
    0.529350;-0.529352;-0.663005;,
    0.144765;-0.719357;-0.679388;,
    -0.165728;-0.165728;-0.972146;,
    -0.165728;0.165728;-0.972146;,
    0.165728;0.165728;-0.972146;,
    0.165728;-0.165728;-0.972146;,
    -0.000001;1.000000;0.000000;,
    0.000000;1.000000;0.000000;,
    0.000000;1.000000;-0.000000;,
    0.000000;1.000000;-0.000001;,
    0.000001;1.000000;0.000000;,
    0.000002;1.000000;0.000000;,
    0.000000;1.000000;0.000001;,
    0.000000;1.000000;0.000001;,
    0.000000;1.000000;0.000000;;
    116;
    3;0,1,2;,
    3;0,2,3;,
    3;0,4,5;,
    3;0,5,6;,
    3;0,6,7;,
    3;0,7,1;,
    3;1,7,8;,
    3;1,8,9;,
    3;1,9,10;,
    3;1,10,2;,
    3;2,10,11;,
    3;2,11,12;,
    3;2,12,13;,
    3;2,13,3;,
    3;3,13,14;,
    3;3,14,15;,
    3;3,15,4;,
    3;3,4,0;,
    3;4,16,17;,
    3;4,17,5;,
    3;5,17,18;,
    3;5,18,6;,
    3;6,18,19;,
    3;6,19,7;,
    3;7,19,20;,
    3;7,20,8;,
    3;8,20,21;,
    3;8,21,9;,
    3;9,21,22;,
    3;9,22,10;,
    3;10,22,23;,
    3;10,23,11;,
    3;11,23,24;,
    3;11,24,12;,
    3;12,24,25;,
    3;12,25,13;,
    3;13,25,26;,
    3;13,26,14;,
    3;14,26,27;,
    3;14,27,15;,
    3;15,27,16;,
    3;15,16,4;,
    3;16,28,29;,
    3;16,29,17;,
    3;17,29,30;,
    3;17,30,18;,
    3;18,30,31;,
    3;18,31,19;,
    3;19,31,32;,
    3;19,32,20;,
    3;20,32,33;,
    3;20,33,21;,
    3;22,34,35;,
    3;22,35,23;,
    3;23,35,36;,
    3;23,36,24;,
    3;24,36,37;,
    3;24,37,25;,
    3;25,37,38;,
    3;25,38,26;,
    3;26,38,39;,
    3;26,39,27;,
    3;27,39,28;,
    3;27,28,16;,
    3;28,40,41;,
    3;28,41,29;,
    3;29,41,42;,
    3;29,42,30;,
    3;30,42,43;,
    3;30,43,31;,
    3;31,43,44;,
    3;31,44,32;,
    3;32,44,45;,
    3;32,45,33;,
    3;33,45,46;,
    3;33,46,34;,
    3;34,46,47;,
    3;34,47,35;,
    3;35,47,48;,
    3;35,48,36;,
    3;36,48,49;,
    3;36,49,37;,
    3;37,49,50;,
    3;37,50,38;,
    3;38,50,51;,
    3;38,51,39;,
    3;39,51,40;,
    3;39,40,28;,
    3;40,52,41;,
    3;41,52,42;,
    3;42,52,53;,
    3;42,53,43;,
    3;43,53,44;,
    3;44,53,45;,
    3;45,53,54;,
    3;45,54,46;,
    3;46,54,47;,
    3;47,54,48;,
    3;48,54,55;,
    3;48,55,49;,
    3;49,55,50;,
    3;50,55,51;,
    3;51,55,52;,
    3;51,52,40;,
    3;55,54,53;,
    3;55,53,52;,
    3;56,56,56;,
    3;57,57,57;,
    3;58,58,58;,
    3;59,59,59;,
    3;60,60,60;,
    3;61,61,61;,
    3;62,62,62;,
    3;63,63,63;,
    3;64,64,64;,
    3;64,64,64;;
   }

   MeshMaterialList  {
    1;
    116;
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0;
    { PDX03_-_Default }
   }

   MeshTextureCoords  {
    86;
    0.500000;0.500000;,
    0.500000;0.500000;,
    0.500000;0.500000;,
    0.500000;0.500000;,
    0.010000;0.010000;,
    0.990000;0.010000;,
    0.010000;0.990000;,
    0.990000;0.990000;,
    0.010000;0.002929;,
    0.005000;0.005000;,
    0.002929;0.010000;,
    0.002929;0.990000;,
    0.005000;0.995000;,
    0.010000;0.997071;,
    0.990000;0.997071;,
    0.995000;0.995000;,
    0.997071;0.990000;,
    0.997071;0.010000;,
    0.995000;0.005000;,
    0.990000;0.002929;,
    0.010000;0.000000;,
    0.002929;0.002929;,
    0.000000;0.010000;,
    0.000000;0.990000;,
    0.002929;0.997071;,
    0.010000;1.000000;,
    0.990000;1.000000;,
    0.997071;0.997071;,
    1.000000;0.990000;,
    1.000000;0.010000;,
    0.997071;0.002929;,
    0.990000;0.000000;,
    0.000000;1.000000;,
    0.001903;1.000000;,
    0.003807;1.000000;,
    0.247513;1.000000;,
    0.249417;1.000000;,
    0.251320;1.000000;,
    0.495026;1.000000;,
    0.496930;1.000000;,
    0.498833;1.000000;,
    0.742540;1.000000;,
    0.744443;1.000000;,
    0.746346;1.000000;,
    0.010000;0.997071;,
    0.005000;0.995000;,
    0.002929;0.990000;,
    0.002929;0.010000;,
    0.005000;0.005000;,
    0.010000;0.002929;,
    0.990000;0.002929;,
    0.995000;0.005000;,
    0.997071;0.010000;,
    0.997071;0.990000;,
    0.995000;0.995000;,
    0.990000;0.997071;,
    0.990000;0.010000;,
    0.010000;0.010000;,
    0.990000;0.990000;,
    0.010000;0.990000;,
    0.000000;0.000000;,
    0.001903;0.000000;,
    0.003807;0.000000;,
    0.247513;0.000000;,
    0.249417;0.000000;,
    0.251320;0.000000;,
    0.495026;0.000000;,
    0.496930;0.000000;,
    0.498833;0.000000;,
    0.742540;0.000000;,
    0.744443;0.000000;,
    0.746346;0.000000;,
    1.000000;1.000000;,
    1.000000;0.000000;,
    0.010000;1.000000;,
    0.002929;0.997071;,
    0.000000;0.990000;,
    0.000000;0.010000;,
    0.002929;0.002929;,
    0.010000;0.000000;,
    0.990000;0.000000;,
    0.997071;0.002929;,
    1.000000;0.010000;,
    1.000000;0.990000;,
    0.997071;0.997071;,
    0.990000;1.000000;;
   }
  }
 }
}

Frame Object05 {
 

 FrameTransformMatrix {
  1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,-0.199000,1.000000;;
 }

 Mesh  {
  4;
  -0.980000;0.980000;0.400000;,
  0.980000;0.980000;0.400000;,
  -0.980000;-0.980000;0.400000;,
  0.980000;-0.980000;0.400000;;
  2;
  3;0,2,3;,
  3;0,3,1;;

  MeshNormals  {
   1;
   0.000000;0.000000;1.000000;;
   2;
   3;0,0,0;,
   3;0,0,0;;
  }

  MeshMaterialList  {
   1;
   2;
   0,
   0;
   { PDX01_-_Default }
  }

  MeshTextureCoords  {
   4;
   0.999500;0.000500;,
   0.000500;0.000500;,
   0.999500;0.999500;,
   0.000500;0.999500;;
  }
 }
}

Frame Object06 {
 

 FrameTransformMatrix {
  1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,-0.201000,1.000000;;
 }

 Mesh  {
  4;
  0.980000;-0.980000;0.000000;,
  -0.980000;-0.980000;0.000000;,
  0.980000;0.980000;0.000000;,
  -0.980000;0.980000;0.000000;;
  2;
  3;2,0,1;,
  3;2,1,3;;

  MeshNormals  {
   1;
   0.000000;0.000000;-1.000000;;
   2;
   3;0,0,0;,
   3;0,0,0;;
  }

  MeshMaterialList  {
   1;
   2;
   0,
   0;
   { PDX01_-_Default }
  }

  MeshTextureCoords  {
   4;
   0.990000;0.010000;,
   0.010000;0.010000;,
   0.990000;0.990000;,
   0.010000;0.990000;;
  }
 }
}

Frame Object07 {
 

 FrameTransformMatrix {
  1.000000,-0.000000,0.000000,0.000000,-0.000000,-1.000000,-0.000000,0.000000,-0.000000,-0.000000,1.000000,0.000000,1.632000,-0.001000,-0.200000,1.000000;;
 }

 Frame {
  

  FrameTransformMatrix {
   1.000000,0.000000,-0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,-0.000000,0.000000,1.000000;;
  }

  Mesh  {
   5;
   -0.920000;1.000000;0.304000;,
   -0.920000;1.000000;0.096000;,
   -0.712000;1.000000;0.096000;,
   -0.712000;1.000000;0.119577;,
   -0.712000;1.000000;0.304000;;
   3;
   3;3,1,2;,
   3;4,0,3;,
   3;3,0,1;;

   MeshNormals  {
    1;
    0.000000;1.000000;0.000000;;
    3;
    3;0,0,0;,
    3;0,0,0;,
    3;0,0,0;;
   }

   MeshMaterialList  {
    1;
    3;
    0,
    0,
    0;
    { PDX03_-_Default }
   }

   MeshTextureCoords  {
    5;
    0.999501;0.000499;,
    0.999501;0.999501;,
    0.000499;0.999501;,
    0.000499;0.886264;,
    0.000499;0.000499;;
   }
  }
 }
}

Frame Object08 {
 

 FrameTransformMatrix {
  1.000000,0.000000,0.000000,0.000000,0.000000,-1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,-0.208000,-0.001000,-0.200000,1.000000;;
 }

 Mesh  {
  5;
  0.920000;1.000000;0.304000;,
  0.920000;1.000000;0.096000;,
  -0.712000;1.000000;0.096000;,
  -0.712000;1.000000;0.119577;,
  -0.712000;1.000000;0.304000;;
  3;
  3;0,4,3;,
  3;3,2,1;,
  3;0,3,1;;

  MeshNormals  {
   1;
   0.000000;1.000000;0.000000;;
   3;
   3;0,0,0;,
   3;0,0,0;,
   3;0,0,0;;
  }

  MeshMaterialList  {
   1;
   3;
   0,
   0,
   0;
   { PDX03_-_Default }
  }

  MeshTextureCoords  {
   5;
   0.000499;0.000500;,
   0.000499;0.999501;,
   0.999501;0.999501;,
   0.999501;0.886264;,
   0.999501;0.000500;;
  }
 }
}