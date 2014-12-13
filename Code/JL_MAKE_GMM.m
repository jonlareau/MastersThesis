function [mix, options, errlog] = JL_MAKE_GMM(feats, ncentres,itr)
%Make a GMM to describe the distribution given by <feats> using <ncentres>
%mixture components, and only itr EM iterations...
%
%Adapted from NETLAB demo by: 
%Jonathan Lareau - Rochester Insititute of Technology - 2006
%programming@jonlareau.com

[inputdim,M] = size(feats);
if nargin < 3
    itr = 25
end
%NetLab GMM

mix = gmm(inputdim, ncentres, 'diag');

options = foptions; 
options(1) = 1; % Prints out error values. 
%options(3) = .1; %min change in log-loklihood to proceed
options(14) = 500; % Max. number of iterations.

disp('Initializing GMM using gmminit');
mix = gmminit(mix, feats', options); %Initialize with k-means

options(1) = 1; % Prints out error values. 
options(5) = 1; % Prevent Covar values from collapsing...
options(14) = itr; % Max. number of iterations.

disp('Running EM for mixture model');
[mix, options, errlog] = gmmem(mix, feats', options); 

%x = testingdata;
%prob = gmmprob(mix, x)