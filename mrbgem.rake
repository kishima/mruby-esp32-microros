MRuby::Gem::Specification.new('mruby-esp32-microros') do |spec|
  spec.license = 'MIT'
  spec.authors = 'Katsuhiko Kageyama'

  spec.cc.include_paths << "#{build.root}/src"

  #spec.add_dependency 'mruby-numeric-ext'
end
