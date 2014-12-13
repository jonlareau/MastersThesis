function out = KL(gmm1,gmm2,x)
%Get the approximate asymetric KL-divergence between two GMM's using
%the feature vectors contained in x
%
%
%Written by: 
%Jonathan Lareau - Rochester Insititute of Technology - 2006
%programming@jonlareau.com

out = 0;
if ((nargin < 3) || (length(x)==1))
    if nargin < 3
        SampSize = 5000; %Default Sample Size
    else 
        SampSize = x;
    end
    x = gmmsamp(gmm1,SampSize);
else

fx = gmmprob(gmm1,x);
gx = gmmprob(gmm2,x);
out = 1/SampSize*sum(log(fx./gx));
