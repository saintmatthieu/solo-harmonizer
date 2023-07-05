mode = 'Release';
groundTruths = readlines('C:/Users/saint/git/github/saintmatthieu/solo-harmonizer/saint/_assets/Les_Petits_Poissons-index-ground-truths.txt');
execinfo = dir(['C:/Users/saint/git/github/saintmatthieu/solo-harmonizer/build/saint/SoloHarmonizer/' mode '/SoloHarmonizerTests.exe']);
elapsedSinceExecModified = datetime - execinfo.date;
startTimes = [];
for l = 1:size(groundTruths, 1)
    nums = str2num(groundTruths(l));
    if size(nums) == 0
        break
    end
    startTimes(end+1) = nums(1);
end
startTimes(end+1) = Inf;
L = 10;
llhThatItStays = linspace(0.5, 0.99, L);
T = 10;
transitionToNextLlh = linspace(0.5, 0.99, T);
O = 10;
observationLlhWeight = linspace(0.001, 0.01, O);

errorPcts = zeros(L, T, O);
sampsPerBlock = 512;
sampsPerSec = 44100;
secsPerBlock = sampsPerBlock/sampsPerSec;
count = 0;
minErr = Inf;
bestL = 0;
bestT = 0;
bestO = 0;
for l = 1:L
    lval = llhThatItStays(l);
    for t = 1:T
        tval = transitionToNextLlh(t);
        for o = 1:O
            oval = observationLlhWeight(o);
            waitbar(count / (L*T*O));
            count = count+1;
            writelines([{num2str(lval)}, {num2str(tval)}, {num2str(oval)}], 'C:/Users/saint/Downloads/params.txt');
            assert(system(['cd C:\\Users\\saint\\git\\github\\saintmatthieu\\solo-harmonizer & .\\build\\saint\\SoloHarmonizer\\' mode '\\SoloHarmonizerTests.exe --gtest_filter=SoloHarmonizerTest.Les_Petits_Poissons']) == 0)
            results = str2double(readlines('C:/Users/saint/Downloads/output.txt'));
            correct = nan;
            fid = fopen('C:/Users/saint/Downloads/debug-labels.txt', 'w');
            errorCount = 0;
            for k = 1:length(results)
                s = secsPerBlock * (k-1);
                result = results(k);
                if result == -1
                    correct = nan;
                    continue;
                end
                truth = find(s < startTimes, 1) - 2;
                if result ~= truth
                    errorCount = errorCount + 1;
                end
                if isnan(correct) | (correct ~= (result == truth))
                    fprintf(fid, ['%f\t%f\tcorrect=' num2str(result==truth) '\n'], s, s);
                end
                correct = result == truth;
            end
            errorCount = errorCount * 100 / sum(results ~= -1);
            if errorCount < minErr
                bestL = l;
                bestT = t;
                bestO = o;
                minErr = errorCount;
            end
            errorPcts(l, t, o) = errorCount;
            fclose(fid);
        end
    end
end
flat = reshape(errorPcts, L*T*O, 1);
[~, i] = min(flat);

% bestL = mod((i-1), L) + 1;
% bestT = mod(floor((i-1)/L), T) + 1;
% bestO = floor((i-1)/L/T) + 1;

bestLlhThatItStays = llhThatItStays(bestL);
bestTransitionToNextLlh = transitionToNextLlh(bestT);
bestObservationLlhWeight = observationLlhWeight(bestO);
% surf(llhThatItStays*ones(1, T),ones(L, 1)*transitionToNextLlh, errorPcts)
% xlabel("llhThatItStays")
% ylabel("transitionToNextLlh")
% xticks(llhThatItStays)
% yticks(transitionToNextLlh)