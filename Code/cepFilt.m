function [xCout, xHout, xFout, xout] = cepFilt(x,nfft,fs,win,ov, minHz, PRINT)
%Do Cepstral Speech Enhancement on mono audio signal <x>
%<nfft>     - Size FFFT to use.
%<fs>       - Sampling Frequency
%<win>      - window size
%<ov>       - Overlap   
%<minHz>    - minimum frequency (LPF cutoff)
%<PRINT>    - Display...
%
%Written by: 
%Jonathan Lareau - Rochester Insititute of Technology - 2006
%programming@jonlareau.com

%dh = 'D:\Jons Files\Thesis\JJL_MS_Thesis\Language_Samples\Hindi\hindi_spkr_4.wav';
%de = 'D:\Jons Files\Thesis\JJL_MS_Thesis\Language_Samples\English\eng_spkr_1.wav';

%[x,fs] = wavread(x);
%For Debugging...
if nargin < 1
    PRINT = 1;
    nfft = 256;
    win = 256;
    ov = 237;
    minHz = 300;
    de = 'D:\Jons Files\Test Data\OGI_TEST_SETS\Train\ENGLISH\EN003DOW.waV'
    %de = 'D:\Jons Files\Test Data\KalmanFilteringSpeechEnhancement\Orig.wav';
    %    de = 'D:\Jons Files\Thesis\JJL_MS_Thesis\Language_Samples\English\Full\eng_spkr_1.wav';
    [x,fs] = wavread(de);
    x = resample(x,8000,fs);
    %x = x(.37*8000:1.3*8000);
    fs = 8000;
else

    if nargin < 6
        minHz = 300;
    end

    if nargin < 7
        PRINT = 0;
    end
end

mixFH = .85 ;
CepThresh = .9;
GaussAlpha = 5 ;

x = x(:);
if (minHz > 0)
    [B,A] = butter(3,minHz/(fs/2),'high');
    x = filtfilt(B,A,x);
end
x = x / max(abs(x(:)));

%Get the unfiltered spectrogram
XE = makeframes(x, win, ov, 'hamming');
fe = fft(XE,nfft);
fe = fe / (max(fe(:))+eps);

b = fe(1:(nfft/2+1),:);
f = 1:fs/(2*(nfft/2+1)):fs/2;
t = 0:((length(x)/fs)/size(b,2)):(length(x)/fs);

%Compute the Cepstrum...
ce = real(ifft(20*log10(abs(fe)), nfft));

szce2 = size(ce,1);
T = -floor(szce2/2):floor(szce2/2)-1;
ind = round((1*fs)/length(x) * size(ce,2));
size(T);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Filter...
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
frm = ce;
%Create a gaussian window to isolate the formants
w = gausswin(size(ce,1),GaussAlpha);
frmF = fftshift(fftshift(ce).*(w(:,ones(1,size(ce,2))) ));

%Subtract the isolated formants from the full Cepstrum to find pitch peaks
frmGm = frm - frmF;
%Find the maximums
[frmLmX, frmLmY] = localmax(frmGm);
%Keep only those peaks that are greater than the threshold
frmH = frmGm.*(frmGm.*frmLmY > CepThresh);

CepCombined = frmH+frmF; %Apporximate 'Clean' Cepstrum...

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Invert back into fourier spectrum...
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Make minimum phase...
w = [1; 2*ones(nfft/2-1,1); ones(1 - rem(nfft,2),1); zeros(nfft/2-1,1)];
w = w(:,ones(1,size(CepCombined,2)));
%Compute FFT
frme = (fft(CepCombined.*w,nfft));
frmeF = (fft(frmF.*w,nfft));
frmeH = (fft(frmH.*w,nfft));
%Undo the logarithmic operation
fbout = (10.^((frme(1:nfft/2+1,:))/20));
fbFout = (10.^((frmeF(1:nfft/2+1,:))/20));
fbHout = (10.^((frmeH(1:nfft/2+1,:))/20));

%fbFHout = fbFout.*fbHout;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Heuristic final Filtering step...just seems to work/sound better...
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
fbCout = fbout - (mixFH*fbFout); %A Cleaned Version

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Get the phase from the original input
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
bout = fe(1:nfft/2+1,:); %Original Spectrogram
pbout = angle(bout); %Phase

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Add back the original phase from the input...
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
fbout = abs(fbout).*exp(pbout*sqrt(-1));%Output without
fbFout = abs(fbFout).*exp(pbout*sqrt(-1));%Formant Envelope
fbHout = abs(fbHout).*exp(pbout*sqrt(-1));%Exitation (Pitch/White Noise)
fbCout = abs(fbCout).*exp(pbout*sqrt(-1));%Cleaned

%fbFHout = abs(fbFHout).*exp(pbout*sqrt(-1));%Formant Env .* Harm Env

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Invert the Spectrogram's
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
xout = ispecgram(fbout,nfft,fs,win,ov);
xout = xout / max(abs(xout(:)));            %Output witout mix subtraction
xFout = ispecgram(fbFout,nfft,fs,win,ov);
xFout = xFout / max(abs(xFout(:)));         %Formant Component Output
xHout = ispecgram(fbHout,nfft,fs,win,ov);
xHout = xHout / max(abs(xHout(:)));         %Harmonic Component Output
xCout = ispecgram(fbCout,nfft,fs,win,ov);
xCout = xCout / max(abs(xCout(:)));         %Enhanced Signal

%xFHout = ispecgram(fbFHout,nfft,fs,win,ov);
%xFHout = xFHout / max(abs(xFHout(:)));

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Display output if Necessary...
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if PRINT
    %Display
    subplot(2,1,1),
    imagesc(t,f,20*log10(abs(bout)+eps));axis xy; title('Input');
    ca = caxis; colorbar;
    %caxis([-60,0]); colorbar;

    subplot(2,1,2),
    imagesc(t,f,20*log10(abs(fbCout)+eps)); axis xy;  title('fbCout');
    caxis([ca(1),ca(2)]); colorbar;
    %caxis([-60,0]); colorbar;
    drawnow;

    disp('Press a key to play input wav file');
    pause();
    wavplay(x,fs);
    disp('Press a key to play Reconstructed - w*G wav file');
    pause();
    wavplay(xCout,fs);
end

%Other Debugging output...
%{
subplot(4,1,3),
imagesc(t,f,20*log10(abs(fbFHout)+eps));axis xy; title('fbFout.*fbHout');
caxis([ca(1),ca(2)]); colorbar;
%caxis([-60,0]); colorbar;

subplot(4,1,4),
imagesc(t,f,20*log10(abs(fbFout)+eps)); axis xy;  title('fBGout');
caxis([ca(1),ca(2)]); colorbar;
%caxis([-60,0]); colorbar;
%}

%}
