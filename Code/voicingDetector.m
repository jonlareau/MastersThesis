function [Voicing, VoicingMax,timeLbls] = voicingDetector(in, fs, PRINT, mAvgPwr, DNWRD_EXPANSION,nfft,win,ov);
%Basically is a downward expanding speech/non-speech detector that uses the
%power spectrum of the signal to label sections as speech or non-speech. 
%
% Eventually want to include voiced/unvoiced detection as well using the
% LPC residual or cepstral analysis, but that is currently not perfected
% and is left for future work.  For right now this uses just a power based
% threshold and the optional downward (forward+backward) expansion.
%
% INPUTS:
% in        - input mono audio signal vector
% fs        - sampling frequency.  If fs==[] or nargin==1, the algorithm
%               assumes that the input <IN> is already a two dimensional
%               spectrogram, otherwise the algorithm treats <IN> as and
%               audio signal vector and uses the specgram() function from
%               the signal processing toolbox to estimate the short time
%               fourier transform of the signal with parameters
%               <fs>,<nfft>,<win>, and <ov>.
% PRINT     - [(0)|1] Weather or not to show output.
% mAvgPwr   - (.00056234 or .01) Speech/non-speech threshold level.
% DNWRD_EXPANSION -[0|(1)] Turn downward Expasion Off/On
% nfft      - (256) NFFT to use in call to specgram()
% win       - (=nfft) WIN to use in call to specgram()
% ov        - (=round(win*.80)) OV to use in call to specgram()
%
% OUTPUTS:
% Voicing  - is an array the same size as the number of frames in the 
%           spectrogram with ones representing speech frames and 0's
%           representing non speech.  (When voicing detection is finally
%           added, voiced frames will == 2, unvoiced == 1, and silence ==
%           0)
%
% VoicingMax - is 1 only at locations that are a local maximum of the power
%           signature
% timeLbls  - is the time labels for each frame
%
%
%Written by: 
%Jonathan Lareau - Rochester Insititute of Technology - 2006
%programming@jonlareau.com


if nargin < 3
    PRINT = 0;
end

if nargin < 4
    if (nargin < 5 || DNWRD_EXPANSION)
        mAvgPwr = .01;
    else
        mAvgPwr = .00056234; 
    end
end

if nargin < 5
    DNWRD_EXPANSION = 1;
end

if nargin < 6
    nfft = 256;
end
if nargin <7 
    win = nfft;
end
if nargin <8 
    ov = round(win*.80);
end

if ( (nargin == 1) || isempty(fs) )
    b1 = in;
    freqLbls = 1:size(b1,1);
    timeLbls = 1:size(b1,2);
else
    MAXFS = 8000;
    if MAXFS < fs
        in = resample(in, MAXFS, fs);
        in = in(:)';
        in = in/max(abs(in(:)));
    end
    [b1,freqLbls,timeLbls] = specgram(in,nfft,MAXFS,win,ov);
end

nfft = (size(b1,1)-1)*2;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Find those frames that have enough signal power to justify.  We start by
%finding the Average power for each frame.  We then find the frames that
%are a local maximum and are above a threshold.  We then use downward
%expansion to expand those peaks down to their connecting local minimum.
%That way, even if the onset of the speech segment is below the threshold
%power level, we should be able to still capture it.  

%calculate short time signal power...
stp(:,1) = abs(b1(:,1)).^2;
AP = mean(abs(b1).^2);

%low pass filter the power signature...
[b,a] = butter(3,1/2,'low');
AvgPwr = filter(b,a,AP);
AvgPwr = AvgPwr / max(AvgPwr(:));

lmnAvgPwr = localMin(AvgPwr);
lmxAvgPwr = localmax(AvgPwr);

%Initialize
Voicing = AvgPwr(1) > mAvgPwr;

%Find Max Voicing Positions
VoicingMax = lmxAvgPwr.*AvgPwr > mAvgPwr;

if DNWRD_EXPANSION
    %Go Forwards and backwards across the frames to find the points where the
    %power is above the threshold, and expand those regions out to their
    %nearest local minima.  Acts as a sort of downward expanding noise
    %gate.
    
    %Forward
    for i = 2:size(b1,2)
        if Voicing(i-1) == 0
            Voicing(i) = AvgPwr(i) > mAvgPwr;
        else
            if (lmnAvgPwr(i)==1 && AvgPwr(i) < mAvgPwr)
                Voicing(i) = 0;
            else
                Voicing(i) = 1;
            end
        end
    end
    
    %
    %Backward
    for i = size(b1,2)-1:-1:2
        if Voicing(i+1) == 0
        else
            if (lmnAvgPwr(i)==1 && AvgPwr(i) < mAvgPwr)
                Voicing(i) = 0;
            else
                Voicing(i) = 1;
            end
        end
    end
    %}
    
else
    %Just use Simple Thresholding
    Voicing = AvgPwr > mAvgPwr;
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%FUTURE WORK:
%Would like to add using the real Cepstrum to determine if each of the
%frames with significant power are voiced or un-voiced.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%Forward-pass filter to remove sporadic false negatives
for i = 2:size(b1,2)-1
    if Voicing(i-1) == Voicing(i+1)
        Voicing(i) = Voicing(i-1);
    end
end

%Display output...
if (nargout == 0 || PRINT ==1)
    hold off; 
    subplot(2,1,1), imagesc(timeLbls,freqLbls,20*log10(abs(b1)+eps)); axis tight; axis xy;
    title('Input Speech File'); xlabel('Time (s)'); ylabel('Frequency (Hz)');
    hold on; 
    plot(timeLbls, Voicing*1000, 'k'); hold off;
    
    subplot(2,1,2), plot(20*log10(abs(AvgPwr+eps))); axis tight; hold on;
    plot(20*log10(abs(ones(size(AvgPwr))*mAvgPwr)), 'r:'); axis tight;
    %stem(20*log10(Voicing+eps), 'k:', 'Marker', 'None'); axis tight;
    hold off;
    title('Average Power'); xlabel('Frame'); ylabel('Decibels (dB)');
    ax = axis;
    axis([ax(1),ax(2),-80,0]);
    drawnow;
end