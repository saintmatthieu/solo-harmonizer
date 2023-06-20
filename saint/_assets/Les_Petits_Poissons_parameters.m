groundTruths = readlines('C:/Users/saint/git/github/saintmatthieu/solo-harmonizer/saint/_assets/Les_Petits_Poissons-index-ground-truths.txt');
startTimes = [];
for i = 1:size(groundTruths, 1)
    nums = str2num(groundTruths(i));
    if size(nums) == 0
        break
    end
    startTimes(end+1) = nums(1);
end
mode = 'Debug';
M = 1;
llhThatItStays = 1 - logspace(log10(0.01), log10(0.5), M);
N = 1;
transitionToNextLlh = [.9763];%1- logspace(-1, -2, N);
errorCounts = zeros(M, N);
sampsPerBlock = 512;
sampsPerSec = 44100;
secsPerBlock = sampsPerBlock/sampsPerSec;
for i = 1:M
    p = llhThatItStays(i);
    for j = 1:N
        q = transitionToNextLlh(j);
        writelines([{num2str(p)}, {num2str(q)}], 'C:/Users/saint/Downloads/params.txt');
        assert(system(['cd C:\\Users\\saint\\git\\github\\saintmatthieu\\solo-harmonizer & .\\build\\saint\\SoloHarmonizer\\' mode '\\SoloHarmonizerTests.exe --gtest_filter=SoloHarmonizerTest.Les_Petits_Poissons']) == 0)
        results = str2double(readlines('C:/Users/saint/Downloads/output.txt'));
        correct = nan;
        fid = fopen('C:/Users/saint/Downloads/debug-labels.txt', 'w');
        for k = 1:length(results)
            t = secsPerBlock * (k-1);
            result = results(k);
            if result == -1, continue; end
            truth = find(t < startTimes, 1) - 2;
            if result ~= truth
                errorCounts(i, j) = errorCounts(i,j)+1;
            end
            if isnan(correct) | (correct ~= (result == truth))
                fprintf(fid, ['%f\t%f\tcorrect=' num2str(result==truth) '\n'], t, t);
                correct = result == truth;
            end
        end
        fclose(fid);
    end
end
flat = reshape(errorCounts, M*N, 1);
[~, i] = min(flat);
R = mod(i-1, M)+1;
C = floor((i-1)/N)+1;
bestLlhThatItStays = llhThatItStays(R);
bestTransitionToNextLlh = transitionToNextLlh(C);
