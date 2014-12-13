function [GMMLangs] = JL_LID_Train(TrainDirs,LANGUAGES,...
    MAXFS,NORM_FEATS,func,funcArgs,SecondsOfSpeech,NumMixtures,ItrEM)
% Returns a cell array of Gaussian Mixture Models for each training set.
% <TrainDirs>   Cell array of directory strings.  For each directory, a GMM
%               will be trained using all of the .WAV files in that
%               directory.
% <LANGUAGES>   Cell array of descriptive strings that label each directory
% <MAXFS>       Maximum Sampling Frequency
% <NORM_FEATS>  [0|1] Whiten the feature vectors before training
% <func>        Function pointer to the feature vector extraction function.
% <funcArgs>    Arguments for the feature vector extraction function.
% <SecondsOfSpeech>    Amount of speech (s) to use for training. A value of
%               -1 uses all available frames.  Otherwise, if the 
%               cumulative amount of frames returned by the feature
%               extraction function multiplied by the seconds represented
%               by each frame is greater than SecondsOfSpeech, frames of
%               feature vectors are randomly selected so that the number of
%               retained frames multiplied by the seconds represented
%               by each frame is aproximately equal to SecondsOfSpeech.
%               The retained frames are then used to train the GMM.
% <NumMixtures> Number of mixtures to be used for each GMM.
% <ItrEM>       Number of Iterations of EM algorithm to use.
%
%Written by: 
%Jonathan Lareau - Rochester Insititute of Technology - 2006
%programming@jonlareau.com


%NOTE:  Need to change over to string-value pairs as a way of parameter
%passing...

FID = 1;
LangFeats = [];
%[SUCCESS,MESSAGE,MESSAGEID] = mkdir('MODELS');
if (length(LANGUAGES) ~= length(TrainDirs))
    error('length(LANGUAGES) must be == length(TrainDirs)')
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Get The features for each language training set...
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
for l = 1:numel(TrainDirs)
    d = TrainDirs{l};
    FEATS = [];

    %Get all the feature vectors for this language
    [FEATS, CT, secPFrame] = JL_BACKEND(d,...
        'func',func,'funcArgs',funcArgs,'MAXFS',MAXFS);
    
    if ((CT > SecondsOfSpeech ) && (SecondsOfSpeech ~= -1))
        FEATS = shuffle(FEATS,2);
        FEATS = FEATS(:,1:round(SecondsOfSpeech/secPFrame));
        CT = size(FEATS,2)*secPFrame;
    end

    fprintf(FID,['Cumulative time for ',LANGUAGES{l},' %.0f(s) \n'], CT);
   
    if NORM_FEATS
        %Make Zero Mean...
        mf = mean(FEATS')';
        FEATS = FEATS - mf(:,ones(1,size(FEATS,2)));

        %Divide Through by STD
        sf = std(FEATS')';
        sf(find(sf==0))=1;
        FEATS = FEATS ./ sf(:,ones(1,size(FEATS,2)));
    end

    LangFeats{end+1} = FEATS;
end
fprintf(FID,'\n');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Make and Store Each one of the Models
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
GMMLangs = [];
for i = 1:length(TrainDirs)
    GMMTemp = JL_MAKE_GMM(LangFeats{i},NumMixtures, ItrEM);
    GMMLangs{i} = GMMTemp;
    %save(['MODELS/GMM_LANG_',LANGUAGES{i}],'GMMTemp');
end
