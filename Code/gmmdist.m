function d = gmmdist(GMM1,feat,mode,GMM2,varargin);
%Default GMM distance calculation function. For each feature vector the
%function returns a distnace <d>.  Custom distance functions must follow
%the same parameter passing scheme.
%<GMM1>     Primary Gaussian Mixture Model used to evaluate each vector in
%           <feat>.
%<feat>     Set of feature vectors to evaluate.  [N by M] with N being the
%           number of feature vectors and M being each vector length.
%<mode>     [('PROB') | 'Sym-KL' | 'KL'] 
%           - 'PROB' - Uses the probability of each feature vector
%           falling on GMM1.  This is the most basic and straightforward
%           distance metric. GMM2 is ignored in this mode.
%           - 'KL' - Uses the Kullback-Liebler Divergence to calculate the
%           asymetric distance of the features between GMM1 and GMM2.
%           - 'Sym-KL' - Uses the Kullback-Liebler Divergence to calculate 
%           the symetric distance of the features between GMM1 and GMM2.
%<GMM2>     Secondary Gaussian Mizture Model used for KL distance modes.
%<varargin> Is ignored in this default distance function.  Intended so that
%           custom distance functions can pass additional parameters if
%           needed. 
%
%Written by: 
%Jonathan Lareau - Rochester Insititute of Technology - 2006
%programming@jonlareau.com
 

if (nargin < 3 || isempty(mode))
    mode = 'PROB';
end

if (nargin < 4 || isempty(GMM2))
    %If the GMM for the feats is not provided, automatically return the
    %PROB.
    mode = 'PROB';
end

if strcmpi(mode,'Sym-KL')
    %Symmetric KL Divergance
    d = -.5*(KL(GMM1,GMM2)+KL(GMM2,GMM1));
elseif strcmpi(mode,'PROB')
    %Probability distance metric...
    d = log(gmmprob(GMM1,feat));
    %d = log(gmmprob(GMM1,feat)+eps);

elseif strcmpi(mode,'KL')
    %KL-Divergence
    d = -KL(GMM1,GMM2);
else
    error('Dist Mode Not Valid');
end