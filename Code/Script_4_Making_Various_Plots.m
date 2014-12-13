d = 'D:\Jons Files\Test Data\OGI_TEST_SETS\sta\Full\ENGLISH\EN003STA.wav'

nfft = 256;
win = 256;
ov = 128;
minHz = 300;
[x,fs] = wavread(d);

%{
[xCout, xHout, xFout, xout] = cepFilt(x,nfft,fs,win,ov, minHz, PRINT);
%}

%
%{
%Pre-Emphasis...
%
A = [1 -.95];
B = [1];
mix = filter(A,B,x);

xf = makeframes(x,256,200,'hamming');
xp = makeframes(mix,256,200,'hamming');

Xp = fft(xp(:,390),512);
Xp = Xp(1:257);
Xf = fft(xf(:,390),512); 
Xf = Xf(1:257);
f = (0:fs/512:fs/2)';

subplot(3,1,1),plot(f,20*log10(abs(Xp)+eps)), title(sprintf('With Pre-Emphasis (A_p_r_e= %.2f)',A(2)));
axis tight
xlabel('Frequency (Hz)');
ylabel('Amplitude (dB)');
ylim([-40,0]); yl = ylim;
xl = xlim;

subplot(3,1,2),plot(f,20*log10(abs(Xf)+eps)), title('Without Pre-Emphasis');
xlabel('Frequency (Hz)');
ylabel('Amplitude (dB)');
%axis tight
ylim(yl); xlim(xl);

[H,F] = freqz(A,B,256,fs);
subplot(3,1,3),semilogx(F,20*log10(abs(H)+eps)), title('Pre-Emphasis Filter Response');
xlabel('Frequency (Hz)');
ylabel('Amplitude (dB)');
axis tight
ylim([-30 10]);
%}

%Speech Signal and Spectrum
%{
A = [1 .95];
B = [1];
mix = filter(B,A,x);

xf = makeframes(x,256,200,'hamming');
xp = makeframes(mix,256,200,'hamming');

[Xf,f,t] = specgram(x,nfft,fs,win,ov);

subplot(2,1,1),plot(xf(:,390)), title('Speech Segment');
axis tight
xlabel('samples');
ylabel('Amplitude');
subplot(2,1,2),plot(f,20*log10(abs(Xf(:,390))+eps)), title('Fourier Spectrum');
xlabel('Frequency (Hz)');
ylabel('Amplitude (dB)');
%}

%Enhancement...
%{
nfft = 256;
win = 256;
ov = 200;
minHz = 300;
%Clean up the telephone speech...
mix = cepFilt(x,nfft,fs,win,ov,minHz,0);

[Xf,f,t] = specgram(x,nfft,fs,win,ov);
[Xe,f,t] = specgram(mix,nfft,fs,win,ov);

subplot(2,1,1),imagesc(t,f,20*log10(abs(Xe)+eps)), title('With Enhancement');
axis xy; colorbar; 
cl = caxis;
subplot(2,1,2),imagesc(t,f,20*log10(abs(Xf)+eps)), title('Without Enhancement');
axis xy; 
caxis(cl);
colorbar;
%}

%DCT-FFT
%{
xf = makeframes(x,256,200);
[b,f,t] = specgram(x,256,fs,256,200);

dctxf = dct(xf(:,390),256);
dftxf = fft(xf(:,390),512); 
dftxf = dftxf(1:256);

subplot(2,1,1),stem(abs(dctxf/max(abs(dctxf)))), title('DCT');
subplot(2,1,2),stem(abs(dftxf/max(abs(dftxf)))), title('FFT');
%}

%Show Filterbank
%{
[wts,binfreqs] = fft2melmx(1024, 8000, 25);
freqs = 1:8000/1024:8000;
plot(freqs,wts')
xlim([0,4000])
title('Sample 25 Filter Mel-Frequency Filterbank');
xlabel('Frequency (Hz)');
ylabel('Channel Weight');

m = 2595*log10(1+binfreqs/700)
str = [];
for i = 2:numel(m)-1
str = [str; {[num2str(round(m(i))),'(mels)']}];
end
gtext(str)


%}

%Fundamental and Harmonics
%{
FS = 8000;  %Sampling Frequency
f = 100;    %Fundamental Frequency (Hz)
t = 0:1/FS:100/f;
w = 2*pi*100;
hMax = 9;
nfft = 4096;
x1 = sin(w*t);
x2 = .5*sin(2*w*t);
x3 = 1/3*sin(3*w*t);
x4 = 1/4*sin(4*w*t);
x5 = 1/5*sin(5*w*t);

subplot(3,1,1),
plot(t,x1,'b'); axis tight; hold on;
plot(t,x2,'b--'); axis tight;
plot(t,x3,'b.-'); axis tight; hold off;
ylim([-2,2]), xlim([0,1/f]);
legend('Fundamental (1st Harmonic, h=1)','2nd Harmonic (h=2)','3rd Harmonic (h=3)');
title('Plot of: A = (1/h)*sin(w*h*t), w = 2*pi*100');
xlabel('t (sec)');
ylabel('A (Amplitude)');

subplot(3,1,2),
sq = 0;
for h = 1:2:hMax;
    sq = sq + (1/h)*sin(w*t*h);
end
plot(t,sq); 
ylim([-2,2]), xlim([0,1/f]);
title(['Plot of: A = sin(w*t)+(1/3)sin(3*w*t)+(1/5)sin(5*w*t)..., w = 2*pi*100, hMax=',num2str(h)]);
xlabel('t (sec)');
ylabel('A (Amplitude)');

xf = fft(sq,nfft);
xf = xf.*conj(xf)/nfft;
xf = xf/max(xf);
fLabel = FS*(0:nfft/2)/nfft;

subplot(3,1,3),
plot(fLabel,20*log10(xf(1:nfft/2+1)),'b-o'); axis tight;
xlim([0,1000]);
ylim([-45,0]);
title('Spectrum of Reconstructed Square Wave');
ylabel('Amplitude (dB)');
xlabel('Frequency (Hz)');
%}

%Show Specgram
%{
specgram(x(2.5*fs:3.6*fs),256,fs,256,200);

%wavplay(x(2.5*fs:3.6*fs),fs);
title('Spectrogram of "Government Will Be Entitled"');

%X = ([exp(-(0:.1:10)),exp(-(10:-.1:0))]);
%subplot(2,1,1), stem(X);
%subplot(2,1,2), plot(real(ifft(X)));
%}

%Calculate MFCC
%{
win = 256;
ov = 200;
[cepstra,aspectrum,pspectrum] = melfcc(x, fs,'wintime', win/fs,...
        'hoptime',(win-ov)/fs)
%}

%
%Show the Silence Detection
voicingDetector(x, fs, 0, .01, 1, 256, 256, 200);
