@filename = "SampleAI"

task :default do
  system("g++ -Wall -O2 -o codevs #{@filename}.cpp")
end

task :run do
  system("./#{@filename}")
end

task :test do
  system("./#{@filename} < sample.in")
end

task :check do
  system("#{@filename}.exe < sample.in")
end
