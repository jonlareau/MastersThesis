function [bx,by]=localmax(y)
%LOCALMAX(Y) Local Maxima, Peak Detection.
% LOCALMAX(Y) when Y is a vector returns a logical vector the same size as
% Y containing logical True where the corresponding Y value is a local
% maximum, that is where Y(k-1)<Y(k)>Y(k+1).
%
% LOCALMAX(Y) when Y is a matrix returns a logical matrix the same size as
% Y containing logical True where the corresponding Y value is a
% local maxima down each column, i.e., Y(k-1,n)<Y(k,n)>Y(k+1,n).
%
% [BX,BY] = LOCALMAX(Z) when Z is a matrix returns two logical matrices the
% same size as Z containing logical True where the corresponding Z value is
% a logical maxima. BX identifies the maxima across each row, i.e.,
% Z(k,n-1)<Z(k,n)>Z(k,n+1), and BY identifies the local maxima down each
% column, i.e., Z(k-1,n)<Z(k,n)>Z(k+1,n).
%
% When two or more consecutive data points have the same local maxima
% value, the last one is identified. First and last data points are
% returned if appropriate.
%
% See also MAX.

% D.C. Hanselman, University of Maine, Orono, ME 04469
% MasteringMatlab@yahoo.com
% Mastering MATLAB 7
% 2005-12-05

if ~isreal(y)
   error('Y Must Contain Real Values Only.')
end
ry=size(y,1);
isrow=ry==1;
if isrow    % convert to column for now
   y=y(:);
end
if nargout==2
   if isvector(y)
      error('Second Output Argument Not Needed.')
   end
   by=local_getmax(y);
   bx=local_getmax(y')';
else  % one output
   bx=local_getmax(y);
   if isrow
   bx=bx.';
   end
end
%-------------------------------------------------------------------------
function b=local_getmax(y)
infy=-inf(1,size(y,2));
k=sign(diff([infy; y; infy]));
b=logical(diff(k+(k==0))==-2);
