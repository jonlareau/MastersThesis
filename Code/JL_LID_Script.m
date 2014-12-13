
%
%Written by: 
%Jonathan Lareau - Rochester Insititute of Technology - 2006
%programming@jonlareau.com

clear;

tic;

func = @JL_GET_FEATS;  %Default function to use to generate features
funcArgs = [];

TrainDirs = [];
TestDirs = [];
VerDirs = [];

%Should we normalize the features...no real reason to change this, but
%the option is there
NORM_FEATS = 1;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%These flags and varuiables can be changed to alter performance
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
CodeBookSize = 64;      %Number of mixtures to use in GMM
SecondsOfSpeech = 1800; %Cumulative amount of speech to use for training
mode = 'LP-CC';         %['LP-CC' | 'MF-CC' | 'PLP-CC' | UserDefined]
GMMDistMode = 'PROB';   %Distance Metric to use...
USE_CMS = 1;            %Use Cepstral Mean Subtraction for Channel Equalization
ENHANCE = 1;            %Use Enhancement Function (NOTE: make a func ptr later)
PRE_EMPH = 0;           %Use Pre-emphasis filter
VTHRESH = .00056234;    %Voicing Threshold = -65dB
V_MAX = 0;              %Use only the Max Locations in Voicing Detector
VOP = 0;                %Use the Vocal Onset Point
PRINT = 1;              %Display incremental output
USE_DELTA = 0;          %Use Delta Cepstra
USE_RASTA = 0;          %Use Rasta (only applicable when using PLP)
USE_SDC = 1;            %Use Shifted Delta Cepstra...
deltaDistSec = .1920;    %=12 frames at win= 256 ov= 128 fs= 8000
ShiftedDeltaSpacingSec=.048;  %=3 frames at win= 256 ov= 128 fs= 8000
NumShiftedDeltas = 3;   %Number of delta Blocks
numCoeff = 12;          %Number of Cepstral Coefficents to use
numCoeffLP = 12;        %Number of LP Coefficients to use(Valid with 'LP-CC' Mode)
itr = 10;               %Number of GMMEM iterations before breaking...

nfft = 256; win = 256; ov = 128;
minHz = 300; framerate = 100; MAXFS = 8000;

TrainHD = 'D:/Jons Files/Test Data/OGI_TEST_SETS/stb/Train/';
TestHD = 'D:/Jons Files/Test Data/OGI_TEST_SETS/stb/Test/';

LANGUAGES = [];
LANGUAGES{end+1} = 'GERMAN';
LANGUAGES{end+1} = 'ENGLISH';
LANGUAGES{end+1} = 'JAPANESE';
%LANGUAGES{end+1} = 'FRENCH';
%LANGUAGES{end+1} = 'FARSI';
%LANGUAGES{end+1} = 'KOREAN';
%LANGUAGES{end+1} = 'MANDARIN';
%LANGUAGES{end+1} = 'SPANISH';
%LANGUAGES{end+1} = 'TAMIL';
%LANGUAGES{end+1} = 'VIETNAM';
nLANGUAGES = length(LANGUAGES);

for i = 1:nLANGUAGES
    %Closed (training) set...
    TrainDirs{end+1} = [TrainHD,LANGUAGES{i}];

    %And the open set...
    TestDirs{end+1} = [TestHD,LANGUAGES{i}];
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Running the algorithm...
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if isempty(funcArgs)
    try
        funcArgs =  {'nfft',nfft,'win',win,'ov',ov,'minHz',minHz,...
            'numCoeff',numCoeff,'numCoeffLP',numCoeffLP,'USE_RASTA',USE_RASTA,...
            'print',PRINT,'USE_DELTA',USE_DELTA,...
            'VTHRESH',VTHRESH,'Mode',mode, 'V_Max', V_MAX, ...
            'VOP', VOP,'USE_SDC',USE_SDC,'USE_CMS',USE_CMS...
            'ENHANCE',ENHANCE,'PRE_EMPH',PRE_EMPH,...
            'deltaDist_Sec',deltaDistSec,...
            'SDC_Block_Spacing_Sec',ShiftedDeltaSpacingSec,...
            'SDC_Blocks',NumShiftedDeltas...
            };
    catch
        error('You did not supply all necessary arguments');
    end
end

disp('running')

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Train - Make arguments string value pairs
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
[GMMLangs] = JL_LID_Train(TrainDirs,LANGUAGES,...
    MAXFS,NORM_FEATS,func,funcArgs,SecondsOfSpeech,CodeBookSize,itr);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Test - Make Arguments string value pairs
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
[confMat,nFiles,fileList] = JL_LID_Test(TestDirs,LANGUAGES,...
    MAXFS,NORM_FEATS,func,funcArgs,GMMDistMode,GMMLangs);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Print Results to Screen
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
fprintf(1,'\nConfusion Matrix Open Set: \n');
for i = 1:nLANGUAGES
    fprintf(1,'%s (%.0f Files):',LANGUAGES{i}(1:2),nFiles(i));
    for j = 1:size(confMat,2)
        fprintf(1,'\t%2.2f', confMat(i,j));
    end
    fprintf(1,'\n');
end
ad = mean(diag(confMat));
sd  = std(diag(confMat));

fprintf(1,'Avg Diag: ');
fprintf(1,'\t%2.2f\n', ad);
fprintf(1,'STDV Diag: ');
fprintf(1,'\t%2.2f\n', sd);

toc