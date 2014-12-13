function [Y, nrows, ncol] = makeframes(varargin)
%FUNCTION makeframes --
%   Separates a mono input signal into a set of frames.  Output matrix is a
%   MxN.  Where M = the frame length, and sequential frames are stored in
%   the N columns.  To return to a mono signal use the command line 
%   >> orig = y(:);  
%
%   This function is used to make sure that all processes on an input
%   signal that require the signal to be separated into frames will use
%   consistently framed data so that input and output lengths will match.
%
%   Input Arguments:  [SIG, FRAMELEN, OVERLAP, WINFLAG]
%   SIG - the input signal
%   FRAMELEN - the length of each frame, in samples
%   OVERLAP - the amount of overlap between frames, in samples
%       (default 0)
%   WINFLAG - Type of window to apply to the data.  Use 'hamming' for a
%       hamming window.  Use 'none' for no window applied to the
%       data.  'none' is the default.
% 
%Adapted from the specgram.m code by: 
%Jonathan Lareau - Rochester Insititute of Technology - 2006
%programming@jonlareau.com

%

error(nargchk(1,4,nargin))
[msg,x,winlen,noverlap,wflag]=chk(varargin);
error(msg)

x = x/max(x(:));

nx = length(x);
nwind = winlen;
if nx < nwind    % zero-pad x if it has length less than the window length
    x(nwind)=0;  nx=nwind;
end
x = x(:); % make a column vector for ease later
ncol = fix((nx-noverlap)/(nwind-noverlap));
colindex = 1 + (0:(ncol-1))*(nwind-noverlap);
rowindex = (1:nwind)';
if length(x)<(nwind+colindex(ncol)-1)
    x(nwind+colindex(ncol)-1) = 0;   % zero-pad x
end

Y = zeros(nwind,ncol);
nrows = nwind;
Y(:) = x(rowindex(:,ones(1,ncol))+colindex(ones(nwind,1),:)-1);

%If we need to use a hamming window
if strcmp(wflag, 'hamming')
    [M,N] = size(Y);
    w = hamming(M); % hamming window
    w = w(:, ones(1,N)); %make same size as output
    Y = Y.*w; %apply window to data
end

function [msg,x,winlen,noverlap,wflag] = chk(P)
%Parse the varargin values for use in the function
%
msg = [];
x = P{1};

if (length(P) > 1) & ~isempty(P{2})
    %winlen = P{3}*Fs/1000;
    winlen = P{2};
else
    winlen = 256;
end

if length(P) > 2 & ~isempty(P{3})
    noverlap = P{3};
else
    noverlap = 0;
end

if length(P) > 3 & ~isempty(P{4})
    wflag = P{4};
else
    wflag = 'none';
end

% NOW do error checking
if min(size(x))~=1,
    msg = 'Requires vector (either row or column) input.';
end

