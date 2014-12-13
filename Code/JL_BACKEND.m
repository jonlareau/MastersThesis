function [FEATS,t,secPFrame] = JL_BACKEND(d, varargin)
%
%Written by: 
%Jonathan Lareau - Rochester Insititute of Technology - 2006
%programming@jonlareau.com


fn = dir(d);

MAXFS = 8000;
FEATS = [];
t =0;
SecondsOfSpeech = -1; %Unlimited

func = @JL_GET_FEATS;
funcArgs = [];

if nargin > 2
    args = varargin;
    if iscell(args{1})
        args = args{1};
    end
    nargs = length(args);

    %ProbFunction, PFunc_Args, TransMatV, TransMatH, iter, UnObsIdx)
    for i=1:2:nargs
        switch args{i},
            case 'func', func = args{i+1};
            case 'funcArgs', funcArgs = args{i+1};
            case 'MAXFS', MAXFS = args{i+1};
            case 'SecondsOfSpeech', SecondsOfSpeech = args{i+1};
            otherwise,
                error(['invalid argument name ' args{i}]);
        end
    end
end

for i = 1:numel(fn) %for each file in the directory
    fname = fn(i).name;
    if ((length(fname) > 3) && (strcmpi(fname(end-3:end), '.wav')))
        %is a .wav file so we willl process it...
        [mix,fs] = wavread([d,'/',fname]);

        %If necessary resample the .wav file so that it is at the
        %appropriate sampling frequency...
        if (fs > MAXFS)
            mix = resample(mix, MAXFS, fs);
            fs = MAXFS;
        end
        
        [feat, sec, secPFrame] = func(mix,fs,funcArgs);
        FEATS = [FEATS feat];
        t = t+sec; %Total Seconds of speech used for training
    end
end

%Should we limit the number of frames returned.  If so, randomly select
%which frames to keep by shuffling and returning only the first x number of
%frames.  x = round(SecondsOfSpeech/secPFrame)
if ((t > SecondsOfSpeech ) && (SecondsOfSpeech ~= -1))
    FEATS = shuffle(FEATS,2);
    FEATS = FEATS(:,1:round(SecondsOfSpeech/secPFrame));
    t = size(FEATS,2)*secPFrame;
end