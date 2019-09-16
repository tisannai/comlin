# = Rakefile for comlin library.

require 'rake/testtask'

task :default do
    system( "a2x -v --doctype manpage --format manpage --destination-dir man man/comlin.3.txt" )
end
task :clean_test do
    sh "rm -f test/result/*"
    sh "rm -f test/comlin_options"
    sh "rm -f test/comlin_config"
    sh "rm -f test/comlin_subcmd"
    sh "rm -f test/comlin_type_prim"
end
