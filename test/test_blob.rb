require_relative 'helper'

describe 'Adapter' do
   supported_by Swift::DB::Postgres, Swift::DB::Mysql do
    describe 'Storing binary objects' do
      before do
        Swift.db do |db|
          type = db.is_a?(Swift::DB::Postgres) ? 'bytea' : 'blob'
          db.execute %q{drop table if exists users}
          db.execute %Q{create table users(id serial, name text, image #{type}, primary key(id))}
        end
      end
      it 'should store and retrieve image' do
        Swift.db do |db|
          io = File.open(File.dirname(__FILE__) + '/house-explode.jpg')
          db.prepare('insert into users (name, image) values(?, ?)').execute('test', io)

          blob = db.prepare('select image from users limit 1').execute.first[:image]

          io.rewind
          assert_kind_of StringIO, blob

          data = blob.read
          assert_equal Encoding::ASCII_8BIT, data.encoding
          assert_equal io.read.force_encoding('ASCII-8BIT'), data
        end
      end
    end
   end
end
