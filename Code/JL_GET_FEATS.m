function [feat,SecondsOfSpeech,SecondsPerFrame] = JL_GET_FEATS(mix,fs,varargin)
%  Gets the feature vectors for the signal MIX with sampling frequency FS
%  and parameters supplied by the set of string-value pairs given in
%  VARARGIN.
%
%Parameters: Brackets denote an optional parameter.  Parenthesese denote
%           the default setting for that parameter.
%'print'            [(0)|1]  Display incremental output.  
%'nfft'             (256) Size of FFT to use when calculating features
%'win'              (256) Size of Window to use when calculating features
%'ov'               (128) Amount of Overlap between calculation windows
%'minHz'            (300) Minimum frequency to allow.  300Hz is the
%                       standard cutoff frequency for telephone speech high
%                       pass filter.
%'numCoeff'         (12) Number of Cepstral Coefficients to use
%'numCoeffLP'       (12) Number of LP Coefficients to use when calculating
%                       LP-CC features.
%'PRE_EMPH'         [(0)|1] Use Pre-Emphasis Filtering to adjust for
%                       natural ~20dB/decade rolloff of the human voice. 
%'ENHANCE'          [0|(1)] Use Cepstral Speech Enhancement algorithm as a
%                       pre-processor.
%'USE_CMS'          [0|(1)] Use Cepstral Mean Subtraction to try to
%                       mitigate channel effects. 
%'USE_SDC'          [0|(1)] Use Shifted Delta Cepstral Coefficients
%'USE_DELTA'        [(0)|1] Use Delta Coefficients
%'USE_POWER_TERM'   [(0)|1] Use or omit the power (first) cepstral term
%'deltaDist'        ([]) The distance (in feature frames) to use when
%                       calculating delta coefficients.
%'SDC_Block_Spacing' ([]) The distance (in frames) to use between shifted
%                       delta blocks.
%'deltaDist_Sec'    (.1920) The distance (in seconds) to use when
%                       calculating delta coefficients.
%'SDC_Block_Spacing_Sec' (.048) The distance (in seconds) to use between
%                       shifted delta blocks.
%'SDC_Blocks'       (3) The Number of Shifted Delta Blocks to Use
%'LifterExp'        (0) The exponent to use when liftering (i.e. weighting)
%                       the cepstral coefficients 
%'Mode'             [('LP-CC')|'PLP-CC'|'MF-CC'|'CUST'] Feature Calculation
%                       mode to use.  'LP-CC' - Linear Prediction derived
%                       Cepstral Coefficients.  'PLP-CC' - Perceptual
%                       Linear Prediction Cepstral Coefficients.  'MF-CC' -
%                       Mel Frequency Cepstral Coefficients.  'CUST' -
%                       Custom cepstral coefficient derivation function.
%                       Use of the 'CUST' option means one must also define
%                       'CustModeFunction' and 'CustModeFunctionArgs'.     
%'CustModeFunction' ([]) A function handle to a user defined function for
%                       calculating the cepstral coefficients that obey the
%                       following prototype: 
%                           CepCoeffs = FUNC(mix,fs,args)
%'CustModeFunctionArgs' ([]) Supplimental arguments for the custom mode
%                       function.
%'VTHRESH'          (.00056234) Threshold for determining if a
%                       frame is speech/non-speech data. .00056234 is
%                       equivalent to -65dB.
%'V_Max'            [(0)|1] Only use the locations of peaks in the
%                       sequential power signature of feature frames to
%                       extract speech frames.  Greatly reduced the number
%                       of feature frames extracted, but helps to ensure
%                       that only features frames corresponding to actual
%                       speech data are used.  Was implemented for
%                       debugging purposes only.  Not Recommended.
%'VOP'              [(0)|1] Try to use the locations of Vocal Onset Points
%                       to determine which frames are extracted from te
%                       audio signal. Again, was implemented as a debugging
%                       tool and is not recommended.
%
%Written by: 
%Jonathan Lareau - Rochester Insititute of Technology - 2006
%programming@jonlareau.com


lifterexp = 0;  %0 = No Liftering, 1 = Linear weighting ...
NEW = 0; %Use newer LP-CC computation code...
nfft = 256; win =  256; ov = win/2;
minHz = 300;
numCoeff = 12;
numCoeffLP = 12;
USE_PLP = 1;
USE_RASTA = 0;
USE_CMS = 1;
PRINT = 0;
USE_DELTA = 0;
PRE_EMPH = 0;
VTHRESH = .00056234;
USE_V_MAX = 0;
USE_V = 1;
USE_SDC = 0; 
USE_POWER_TERM = 0;
mode = 'LP-CC';
ENHANCE = 1;
deltaDist = [];
ShiftedDeltaSpacing = []; 
NumShiftedDeltas = 3;
CustModeFunction = [];
CustModeFunctionArgs = [];
deltaDistSec = .1920; %3 frames at win= 256 ov= 128 fs= 8000
ShiftedDeltaSpacingSec =.048;  %3 frames at win= 256 ov= 128 fs= 8000

%Added Jul 13th...
%Normalize the input...
%mix = mix / mean(mix(:));
%mix = mix / max(abs(mix(:)));

if (nargin > 2 && ~isempty(varargin))
    args = varargin;
    if iscell(args{1})
        args = args{1};
    end
    nargs = length(args);

    %ProbFunction, PFunc_Args, TransMatV, TransMatH, iter, UnObsIdx)
    for i=1:2:nargs
        switch args{i},
            case 'print', PRINT = args{i+1};
            case 'nfft', nfft = args{i+1};
            case 'win', win = args{i+1};
            case 'ov', ov = args{i+1};
            case 'numCoeff',  numCoeff = args{i+1};
            case 'numCoeffLP',  numCoeffLP = args{i+1};
            case 'USE_RASTA', USE_RASTA = args{i+1} ;
            case 'USE_CMS', USE_CMS = args{i+1} ;
            case 'PRE_EMPH', PRE_EMPH = args{i+1} ;
            case 'ENHANCE', ENHANCE = args{i+1} ;
            case 'USE_SDC', USE_SDC = args{i+1} ;
            case 'USE_DELTA', USE_DELTA = args{i+1};
            case 'deltaDist', deltaDist = args{i+1} ;
            case 'SDC_Block_Spacing',ShiftedDeltaSpacing = args{i+1};
            case 'SDC_Blocks',NumShiftedDeltas = args{i+1};
            case 'LifterExp',lifterexp = args{i+1};
            case 'UseNewer_LP-CC_Code', NEW = args{i+1};
            case 'deltaDist_Sec', 
                deltaDist=[]; 
                deltaDistSec = args{i+1} ;
            case 'SDC_Block_Spacing_Sec',
                ShiftedDeltaSpacing=[]; 
                ShiftedDeltaSpacingSec = args{i+1};
            case 'Mode', mode = args{i+1}; %['LP-CC'|'PLP-CC'|'MF-CC'|'CUST']
            case 'CustModeFunction', CustModeFunction = args{i+1};
            case 'CustModeFunctionArgs', CustModeFunctionArgs = args{i+1};
            case 'minHz',  minHz = args{i+1};
            case 'Print',  PRINT = args{i+1};
            case 'VTHRESH', VTHRESH = args{i+1};
            case 'USE_POWER_TERM', USE_POWER_TERM = args{i+1};
            case 'V_Max', USE_V_MAX = args{i+1};
                if USE_V_MAX
                    USE_VOP = 0;
                    USE_V = 0;
                end
            case 'VOP', USE_VOP = args{i+1};
                if USE_VOP
                    USE_V = 0;
                    USE_V_MAX = 0;
                end
            otherwise,
                error(['invalid argument name ' args{i}]);
        end
    end
end

if isempty(deltaDist)
    deltaDist = max(1,round(deltaDistSec*fs/(win-ov)));
end
if isempty(ShiftedDeltaSpacing)
    ShiftedDeltaSpacing = max(1,round(ShiftedDeltaSpacingSec*fs/(win-ov)));
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Pre-processing
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%Normalize the file...
mix = mix / max(abs(mix(:)));

if ENHANCE
    %Clean up the telephone speech...
    mix = cepFilt(mix,nfft,fs,win,ov,minHz,0);
end

if PRE_EMPH
    %Pre-Emphasis...
    A = [1 -.95];
    B = [1];
    mix = filter(A,B,mix);
end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

NSeconds = length(mix)/fs;  %Length of the Full file (s)
SecondsPerFrame = win/fs;   %Seconds of spech data contained in each frame.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Get the Features for each frame of the audio signal....
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if strcmpi(mode,'PLP-CC')
    %Use Ellis RASTA-MAT package to computer PLP-CC
    [cepCoeffs, spectra, pspectrum, lpcas] = ...
        rastaplp(mix, fs, USE_RASTA, numCoeff, win, ov, lifterexp);
elseif strcmpi(mode,'MF-CC')
    %Use Ellis RASTA-MAT package to computer MF-CC
    [cepCoeffs,aspectrum,pspectrum] = melfcc(mix, fs, 'wintime', win/fs,...
        'hoptime',(win-ov)/fs,'lifterexp',lifterexp,'numcep',numCoeff);
elseif strcmpi(mode,'LP-CC')
    %Turn into frames
    mixf = makeframes(mix,win,ov, 'hamming');

    %Get LP-CC coefficients...
    if NEW  %Uses the Ellis package to compute LP-CC's...
        %Get the Linear Predictive Coefficients
        [lpcas] = dolpc(mixf,numCoeffLP);
        
        %Use Recursion to find Cepstral Coefficients
        cepCoeffs = lpc2cep(lpcas',numCoeff);
        
        %Apply Liftering if needed
        cepCoeffs = lifter(cepCoeffs, lifterexp);  %NEW Consistant with others
    else  %older method, more direct method...
        %Get the Linear Predictive Coefficients
        [lpcas, lpcErrPow] = lpc(mixf,numCoeffLP);
        
        %Get the frequency spectra made from the LPC coefficients using the
        %signa processing toolbox functions...
        pspectrum = zeros(nfft/2+1,size(mixf,2));
        for j = 1:size(mixf,2)
            [B,A] = eqtflength(1,[0 lpcas(j,:)]);
            pspectrum(:,j) = freqz(B,A,nfft/2+1,fs);
        end
        
        %Normalize the spectrum
        pspectrum = pspectrum / max(abs(pspectrum(:)));

        %Convert the Fourier Spectra to N Cepstral Coeff's...
        cepCoeffs = real(ifft(log(abs(pspectrum)), nfft));
        
        %Get the Linearly Weighted (Liftered) Cepstral Coefficients's for
        %each frame, and also only retain the number of
        %coefficients that we want...
        mul = 1:numCoeff;
        if (lifterexp ~= 0)
            mul1 = mul.^lifterexp;
            cepCoeffs = cepCoeffs(mul,:).*mul1(ones(size(cepCoeffs,2),1),:)';
        else
            cepCoeffs = cepCoeffs(mul,:);
        end
    end
else
    if isa(CustModeFunction, 'function_handle')
        %Use a custom feature extraction method...
        cepCoeffs = CustModeFunction(mix,fs,CustModeFunctionArgs);
    else
        error('In order to use the Custom feature calculation mode, you must supply a function handle');
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Post-Processing
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if ~USE_POWER_TERM
    %Omit the power coefficient...
    cepCoeffs = cepCoeffs(2:end,:);
end

%Try to remove channel effects by doing Cepstral Mean Subtraction...
if USE_CMS
    %Might want to make this a little more advanced via sliding window,
    %etc...
    cepCoeffs = cepCoeffs - repmat(mean(cepCoeffs,2),1,size(cepCoeffs,2));
end

feat = [cepCoeffs];

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Figure out which feature frames to remove (silence) and make delta cepstra
%if applicable...
if (USE_V_MAX || USE_V)
    %Do we want to use the Delta Cepstra...
    if (USE_DELTA || USE_SDC)
        %Get the delta-Cepstra
        DeltaCepCoeffs = deltas(cepCoeffs,deltaDist);
        
        %If we need to compute the Shifted-Delta Cepstra...
        if USE_SDC
            %P = shift between blocks
            %k = number of blocks
            %N = numcoeff
            %d = deltaDist
            P = ShiftedDeltaSpacing; 
            k = NumShiftedDeltas;
            feat = [DeltaCepCoeffs];
            for i = 2:k
                shift = (k-1)*P;
                feat = [feat; circshift(DeltaCepCoeffs, [0 -shift])];
            end
            
            %Remove frames that have wrapped around...
            feat = feat(:,1:end-shift);
        else
            feat = [feat; DeltaCepCoeffs]; %add any other additional features here...
        end
    end

    %Locate using power, and get rid of silent frames.  Basing on the
    %derived power spectrum can introduce inconsistancies between feature
    %types, so instead we use the normalized original signal to make
    %speech/non-speech determination...
    %[V1, Vm1, t1] = voicingDetector(pspectrum,[],0,VTHRESH,0);
    [V, Vm, t] = voicingDetector(mix,fs,0,VTHRESH,0);
    
    FrameSecLabels = (1:size(feat,2))*(NSeconds/size(feat,2));
    
    if USE_V_MAX %Not Recommended
        t(find(Vm'==0)) = [];
    else
        %feat(:,find(V==0)) = [];
        t(find(V'==0)) = [];
    end
    %Locate which frames correspond to speech.
    npts = nearestpoint( t, FrameSecLabels);
    
    %Remove any duplicates
    npts = removeDuplicates(npts);

    %only keep those concatonated feature vectors that correspond to the
    %speech segments and disregard the other (silence) frames
    feat = feat(:,npts);
    
elseif USE_VOP
    %We use the location of Vocal onset points, and concatonate subsequent
    %frames of features around the VOP...
    spacing = 1;

    feat = [feat; circshift(feat,[0,1*spacing]);  circshift(feat,[0,-1*spacing]);...
        circshift(feat,[0,-2*spacing]);  circshift(feat,[0,-3*spacing])];

    %Do we want to use the Delta Cepstra...
    if USE_DELTA
        %Get the delta-Cepstra
        DeltaCepCoeffs = deltas(cepCoeffs,deltaDist);

        feat = [feat; DeltaCepCoeffs]; %add any other additional features here...
    end

    %Find Locations of Vocal Onset Points...
    [VOP_Times, VOP, dVOP] = VocalOnsetPoints(mix, fs, win, ov, numCoeffLP);
    FrameSecLabels = (1:size(feat,2))*(NSeconds/size(feat,2));

    %Locate which frames correspond to the Vocal Onset Points.
    npts = nearestpoint( VOP_Times, FrameSecLabels);
    npts = removeDuplicates(npts);

    %only keep those concatonated feature vectors that correspond to the
    %VOP's
    feat = feat(:,npts);
else
    error('Cannot determine which voicing Algorithm to use');
end

%How much speech has been kept?
SecondsOfSpeech = size(feat,2)*SecondsPerFrame;
if PRINT
    disp(['FEATS Size: ',num2str(SecondsOfSpeech),' (s)']);
end