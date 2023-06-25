groundTruths = readlines('C:/Users/saint/git/github/saintmatthieu/solo-harmonizer/saint/_assets/Les_Petits_Poissons-index-ground-truths.txt');
startTimes = [];
for l = 1:size(groundTruths, 1)
    nums = str2num(groundTruths(l));
    if size(nums) == 0
        break
    end
    startTimes(end+1) = nums(1);
end
mode = 'Release';
L = 25;
llhThatItStays = linspace(0.01, 0.99, L)';
T = 25;
transitionToNextLlh = linspace(0.01, 0.99, T);
errorPcts = zeros(L, T);
sampsPerBlock = 512;
sampsPerSec = 44100;
secsPerBlock = sampsPerBlock/sampsPerSec;
count = 0;
for l = 1:L
    lval = llhThatItStays(l);
    for t = 1:T
        waitbar(count / (L*T));
        count = count+1;
        tval = transitionToNextLlh(t);
        writelines([{num2str(lval)}, {num2str(tval)}], 'C:/Users/saint/Downloads/params.txt');
        assert(system(['cd C:\\Users\\saint\\git\\github\\saintmatthieu\\solo-harmonizer & .\\build\\saint\\SoloHarmonizer\\' mode '\\SoloHarmonizerTests.exe --gtest_filter=SoloHarmonizerTest.Les_Petits_Poissons']) == 0)
        results = str2double(readlines('C:/Users/saint/Downloads/output.txt'));
        correct = nan;
        fid = fopen('C:/Users/saint/Downloads/debug-labels.txt', 'w');
        for k = 1:length(results)
            s = secsPerBlock * (k-1);
            result = results(k);
            if result == -1, continue; end
            truth = find(s < startTimes, 1) - 2;
            if result ~= truth
                errorPcts(l, t) = errorPcts(l,t)+1;
            end
            if isnan(correct) | (correct ~= (result == truth))
                fprintf(fid, ['%f\t%f\tcorrect=' num2str(result==truth) '\n'], s, s);
                correct = result == truth;
            end
        end
        errorPcts(l, t) = errorPcts(l, t)*100/length(results);
        fclose(fid);
    end
end
flat = reshape(errorPcts, L*T, 1);
[~, i] = min(flat);
R = mod(i-1, L)+1;
C = floor((i-1)/L)+1;
bestLlhThatItStays = llhThatItStays(R);
bestTransitionToNextLlh = transitionToNextLlh(C);
surf(llhThatItStays*ones(1, T),ones(L, 1)*transitionToNextLlh, errorPcts)
xlabel("llhThatItStays")
ylabel("transitionToNextLlh")
xticks(llhThatItStays)
yticks(transitionToNextLlh)