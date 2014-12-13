function plotGMMContour(mix,RANGES,STEPS,i,j)

if (nargin < 2 || isempty(RANGES))
    RANGES = [-5,5,-5,5];
end
oRANGES = RANGES;
mRANGES = max(abs(round(RANGES)));
RANGES = [-mRANGES,mRANGES,-mRANGES,mRANGES];

if (nargin < 3 || isempty(STEPS))
    STEPS = [.1 .1];
end

mixModel = mix;
mixModel.nin = 2;
mixModel.centres = mix.centres(:,[i j]);
mixModel.covars = mix.covars(:,[i j]);
mixModel.nwts = 5*mix.ncentres;

% Plot the result
x = RANGES(1):STEPS(1):RANGES(2);
y = RANGES(3):STEPS(2):RANGES(4);
[X, Y] = meshgrid(x,y);
X1 = X(:);
Y1 = Y(:);
grid = [X1 Y1];
Z = gmmprob(mixModel, grid);
Z = reshape(Z, length(x), length(y));
Z = Z / max(abs(Z(:)));
%c = mesh(x, y, Z);
[c,h] = contour(X,Y,Z);
%imagesc(x,y,Z);
axis(oRANGES);
drawnow;