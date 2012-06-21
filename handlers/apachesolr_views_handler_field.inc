<?php

class apachesolr_views_handler_field extends views_handler_field {

  function get_value($values, $field = NULL) {
    $alias = isset($field) ? $this->aliases[$field] : $this->field_alias;
    if (isset($values->node->{$alias})) {
      if (is_array($values->node->{$alias})) {
        return implode(" ", $values->node->{$alias});
      }
      else {
        return $values->node->{$alias};
      }
    }
  }

}