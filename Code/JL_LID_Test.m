function [confMat,nFiles,fileList] = JL_LID_Test(varargin)
%Test each of the .wav files in the directories entered against each of the
%stored GMM's for each LANGUAGE.  Arguments are string-value pairs.
% <TestDirs>    Cell array of directory strings.  For each directory, each
%               .wav file in the directory will be tested against each of
%               the GMM's.
% <Languages>   Cell array of descriptive strings that label each GMM.
% <FeatFunc>    Function pointer to the feature vector extraction function.
% <FeatFuncArgs>Arguments for the feature vector extraction function.
% <GMMs>        The Gaussian Mixture Models for each language.
%
%These additional arguments are optional and/or have default preset values:
% <MaxFS>       (8000) Maximum Sampling Frequency
% <NormFeats>   [0|(1)] Whiten the feature vectors before training
% <GMMDistMode> Operating mode of the GMM distance function.('PROB')
% <GMMDistFunc> Function pointer for the distance function used to evaluate
%               the feature vectors on each GMM.  (@gmmdist)
% <GMMDistFuncArgs> Additional Arguments for the GMM distance function.([])
% <GMM-UBM>      (optional) Universal Background Model to use for distance
%               evaluation.
%
%Written by: 
%Jonathan Lareau - Rochester Insititute of Technology - 2006
%programming@jonlareau.com

nFiles = [];
fileList = [];

TestDirs = [];
LANGUAGES = [];
func = [];
funcArgs = [];
GMMLangs = [];
UBM = [];

%Optional Args...
GMMDistMode = 'Prob';
MAXFS = 8000;
NORM_FEATS = 1;
GMMDistFunc = @gmmdist;
GMMDistFuncArgs = [];

if (length(varargin)==8)
    %Older parameter entry
    TestDirs = varargin{1};
    LANGUAGES = varargin{2};
    MAXFS = varargin{3};
    NORM_FEATS = varargin{4};
    func = varargin{5};
    funcArgs = varargin{6};
    GMMDistMode = varargin{7};
    GMMLangs = varargin{8}
    %{
    if (isempty(GMMLangs))
        %Load the GMM Language Models...
        for i = 1:length(LANGUAGES)
            load(['MODELS/GMM_LANG_',LANGUAGES{i}]);
            GMMLangs{i} = gmmTemp;
        end
    end
    %}
else
    args = varargin;
    if iscell(args{1})
        args = args{1};
    end
    nargs = length(args);

    %ProbFunction, PFunc_Args, TransMatV, TransMatH, iter, UnObsIdx)
    for i=1:2:nargs
        switch args{i},
            case 'TestDirs', TestDirs = args{i+1};
            case 'Languages', LANGUAGES = args{i+1};
            case 'MaxFS', MAXFS = args{i+1};
            case 'NormFeats', NORM_FEATS = args{i+1};
            case 'FeatFunc', func = args{i+1};
            case 'FeatFuncArgs', funcArgs = args{i+1};
            case 'GMMDistMode', GMMDistMode = args{i+1}; %Default Prob
            case 'GMM-UBM', UBM = args{i+1};
            case 'GMMs', GMMLangs = args{i+1};
            case 'GMMDistFunc', GMMDistFunc = args{i+1};
            case 'GMMDistFuncArgs', GMMDistFuncArgs = args{i+1};
            otherwise
                error([args{i},' is not a valid option']);
        end
    end
end

if isempty(func)
    func = @JL_GET_FEATS;
    funcArgs = [];
end

if ( isempty(TestDirs) || isempty(LANGUAGES) || isempty(GMMLangs) )
    error('You did not supply enough args');
end
confMat = zeros(length(TestDirs),length(LANGUAGES));

for l = 1:length(TestDirs);
    d = TestDirs{l};
    fn = dir(d);
    FEATS = [];
    for i = 1:numel(fn) %for each file in the directory
        fname = fn(i).name;
        if ((length(fname) > 3) && (strcmpi(fname(end-3:end), '.wav')))
            %is a .wav file so we willl process it...
            [mix,fs] = wavread([d,'/',fname]);
            if (fs > MAXFS)
                mix = resample(mix, MAXFS, fs);
                fs = MAXFS;
            end

            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            %Get the features for this file...
            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            feat = func(mix,fs,funcArgs);
            if NORM_FEATS
                %Make Zero Mean...
                mf = mean(feat')';
                feat = feat - mf(:,ones(1,size(feat,2)));

                %Divide Through by STD
                sf = std(feat')';
                sf(find(sf==0))=1;
                feat = feat ./ sf(:,ones(1,size(feat,2)));
            end

            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            %Score this file against the models for each category
            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            c = zeros(1,length(LANGUAGES)); %confidence values....

            %If using a UBM add functionality for it here....
            %if ~strcmpi(GMMDistMode,'PROB')
            %    UBM = JL_MAKE_GMM(feat,CodeBookSize,itr);
            %else
            %    UBM = [];
            %end
            
            for i = 1:length(LANGUAGES)
                GMM1 = GMMLangs{i};
                c(i) = sum(GMMDistFunc(GMM1,feat',GMMDistMode,UBM,GMMDistFuncArgs));
            end

            %Find the model that has the best score, and save that as the
            %category for this file...
            [Y,ind] = max(c);
            confMat(l,ind) = confMat(l,ind)+1;
            
            %Also output a list of filenames and their detected language
            fileList{end+1,1} = fname;
            fileList{end,2} = LANGUAGES{ind};
        end
    end
    nFiles(l) = sum(confMat(l,:));
    %Turn confusion matrix into percentages (divide each row by its sum)
    confMat(l,:) = 100*confMat(l,:) / nFiles(l);
end
