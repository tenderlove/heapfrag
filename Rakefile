# -*- ruby -*-

require 'rubygems'
require 'hoe'

Hoe.plugins.delete :rubyforge
Hoe.plugin :minitest
Hoe.plugin :gemspec # `gem install hoe-gemspec`
Hoe.plugin :git     # `gem install hoe-git`

gem 'rake-compiler', '>= 0.4.1'
require "rake/extensiontask"

Hoe.spec 'heapfrag' do
  developer('Aaron Patterson', 'tenderlove@ruby-lang.org')
  self.readme_file   = 'README.markdown'
  self.history_file  = 'CHANGELOG.rdoc'
  self.extra_rdoc_files  = FileList['*.rdoc']

  self.spec_extras[:extensions] = ["ext/heapfrag/extconf.rb"]
  Rake::ExtensionTask.new "heapfrag", spec do |ext|
    ext.lib_dir = File.join(*['lib', ENV['FAT_DIR']].compact)
  end
end

# vim: syntax=ruby
