function [VOP_Times, VOP, dVOP] = VocalOnsetPoints(m, fs, win, ov, NCoeff)
%Find the vocal onset points for a mono speech signal <m> with sampling
%frequency <fs> using window <win>, overlap <ov>, and using <NCoeff> LPC
%Coefficients.
%
%Written by: 
%Jonathan Lareau - Rochester Insititute of Technology - 2006
%programming@jonlareau.com

if nargin < 6
    PRINT = 0;
end

NCoeff = 64;
NSamples = length(m);
NSeconds = NSamples / fs;

mix = makeframes(m,win,ov,'hamming');
NFrames = size(mix,2);
FrameLabels = ((1:NFrames)/NFrames)*NSeconds;

[a,g] = lpc(mix,NCoeff);                 %Get LPC Coeffs

est_x = zeros(size(mix));

for i = 1:NFrames
    est_x(:,i) = filter([0 -a(i, 2:end)],1,mix(:,i)); %Estimate the Signal
end

e = mix-est_x;                       %Residual

EnS = sum(abs(mix.^2));     %Energy of Signal
EnR = sum(abs(e.^2));       %Energy of Residual

VOP = EnS./EnR;


dVOP = (circshift(VOP,[0 -1])-circshift(VOP,[0 1]));

%Should we LPF?
[B,A] = butter(3,.09,'low');
dVOP = filtfilt(B,A,dVOP);

dVOP = dVOP / max(dVOP(:));
dVOP(find(dVOP < .1)) = 0;

lmxdVOP = localmax(dVOP);
VOPr = lmxdVOP.*dVOP;
VOP_Times = lmxdVOP.*FrameLabels;
VOP_Times(find(VOP_Times == 0)) = [];

if (PRINT || nargout == 0)
    b = 20*log10(abs(fft(mix,256))+eps);
    b = b(1:127,:);
    imagesc(b), axis xy; hold on;
    %plot(127*dVOP/max(abs(dVOP(:)))); axis tight;
    stem(127*VOPr / max(abs(VOPr(:))),'k','Marker','None','LineWidth',4); hold off;
    drawnow;
end
